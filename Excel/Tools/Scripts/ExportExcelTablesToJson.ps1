param(
	[string]$SourceDir,
	[string]$TargetDir,
	[string]$SheetName = 'GameData',
	[string[]]$WorkbookPaths
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($SourceDir))
{
	$SourceDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
}

if ([string]::IsNullOrWhiteSpace($TargetDir))
{
	$ProjectRoot = Split-Path -Parent $SourceDir
	$TargetDir = Join-Path $ProjectRoot 'Content\Json\Table'
}

Add-Type -AssemblyName System.IO.Compression.FileSystem

function Read-ZipEntryText
{
	param(
		[System.IO.Compression.ZipArchive]$Archive,
		[string]$EntryPath
	)

	$Entry = $Archive.Entries | Where-Object FullName -eq $EntryPath
	if ($null -eq $Entry)
	{
		return $null
	}

	$Reader = [System.IO.StreamReader]::new($Entry.Open())
	try
	{
		return $Reader.ReadToEnd()
	}
	finally
	{
		$Reader.Dispose()
	}
}

function Get-ColumnIndexFromReference
{
	param([string]$CellReference)

	if ([string]::IsNullOrWhiteSpace($CellReference))
	{
		return 0
	}

	$Letters = ($CellReference -replace '[^A-Z]', '')
	$Index = 0
	foreach ($Letter in $Letters.ToCharArray())
	{
		$Index = ($Index * 26) + ([int][char]$Letter - [int][char]'A' + 1)
	}

	return $Index
}

function Get-SharedStringTable
{
	param([System.IO.Compression.ZipArchive]$Archive)

	$SharedStringsText = Read-ZipEntryText -Archive $Archive -EntryPath 'xl/sharedStrings.xml'
	if ([string]::IsNullOrWhiteSpace($SharedStringsText))
	{
		return [System.Collections.Generic.List[string]]::new()
	}

	$SharedStringsXml = [xml]$SharedStringsText
	$MainNs = 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'
	$NamespaceManager = [System.Xml.XmlNamespaceManager]::new($SharedStringsXml.NameTable)
	$NamespaceManager.AddNamespace('a', $MainNs)

	$SharedStrings = New-Object System.Collections.Generic.List[string]
	foreach ($StringItem in $SharedStringsXml.SelectNodes('//a:si', $NamespaceManager))
	{
		$TextNodes = $StringItem.SelectNodes('.//a:t', $NamespaceManager)
		$CombinedText = ($TextNodes | ForEach-Object { $_.InnerText }) -join ''
		$SharedStrings.Add($CombinedText)
	}

	return $SharedStrings
}

function Convert-CellValue
{
	param(
		[System.Xml.XmlNode]$Cell,
		[System.Xml.XmlNamespaceManager]$NamespaceManager,
		[System.Collections.Generic.List[string]]$SharedStrings
	)

	$InlineTextNodes = $Cell.SelectNodes('./a:is//a:t', $NamespaceManager)
	if ($InlineTextNodes.Count -gt 0)
	{
		return ($InlineTextNodes | ForEach-Object { $_.InnerText }) -join ''
	}

	$ValueNode = $Cell.SelectSingleNode('./a:v', $NamespaceManager)
	if ($null -eq $ValueNode)
	{
		return $null
	}

	$RawValue = $ValueNode.InnerText
	$CellType = $Cell.Attributes['t']
	$TypeValue = if ($null -ne $CellType) { $CellType.Value } else { '' }

	switch ($TypeValue)
	{
		's'
		{
			$SharedIndex = 0
			if ([int]::TryParse($RawValue, [ref]$SharedIndex) -and $SharedIndex -ge 0 -and $SharedIndex -lt $SharedStrings.Count)
			{
				return $SharedStrings[$SharedIndex]
			}

			return $RawValue
		}
		'str'
		{
			return $RawValue
		}
		'b'
		{
			return $RawValue -eq '1'
		}
		default
		{
			$IntegerValue = 0L
			if ([long]::TryParse($RawValue, [System.Globalization.NumberStyles]::Integer, [System.Globalization.CultureInfo]::InvariantCulture, [ref]$IntegerValue))
			{
				return $IntegerValue
			}

			$DoubleValue = 0.0
			if ([double]::TryParse($RawValue, [System.Globalization.NumberStyles]::Float, [System.Globalization.CultureInfo]::InvariantCulture, [ref]$DoubleValue))
			{
				return $DoubleValue
			}

			return $RawValue
		}
	}
}

function Get-WorksheetTargetPath
{
	param(
		[xml]$WorkbookXml,
		[xml]$RelationshipsXml,
		[string]$WorksheetName
	)

	$MainNs = 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'
	$RelNs = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
	$PkgNs = 'http://schemas.openxmlformats.org/package/2006/relationships'

	$WorkbookNsManager = [System.Xml.XmlNamespaceManager]::new($WorkbookXml.NameTable)
	$WorkbookNsManager.AddNamespace('a', $MainNs)
	$WorkbookNsManager.AddNamespace('r', $RelNs)

	$RelationshipsNsManager = [System.Xml.XmlNamespaceManager]::new($RelationshipsXml.NameTable)
	$RelationshipsNsManager.AddNamespace('a', $PkgNs)

	$SheetNode = $WorkbookXml.SelectSingleNode("//a:sheets/a:sheet[@name='$WorksheetName']", $WorkbookNsManager)
	if ($null -eq $SheetNode)
	{
		return $null
	}

	$RelationshipId = $SheetNode.GetAttribute('id', $RelNs)
	if ([string]::IsNullOrWhiteSpace($RelationshipId))
	{
		return $null
	}

	$RelationshipNode = $RelationshipsXml.SelectSingleNode("//a:Relationship[@Id='$RelationshipId']", $RelationshipsNsManager)
	if ($null -eq $RelationshipNode)
	{
		return $null
	}

	return 'xl/' + $RelationshipNode.Target
}

function Convert-WorksheetToRows
{
	param(
		[xml]$WorksheetXml,
		[System.Collections.Generic.List[string]]$SharedStrings
	)

	$MainNs = 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'
	$NamespaceManager = [System.Xml.XmlNamespaceManager]::new($WorksheetXml.NameTable)
	$NamespaceManager.AddNamespace('a', $MainNs)

	$RowNodes = $WorksheetXml.SelectNodes('//a:sheetData/a:row', $NamespaceManager)
	if ($RowNodes.Count -eq 0)
	{
		return @()
	}

	$ParsedRows = New-Object System.Collections.Generic.List[hashtable]
	foreach ($RowNode in $RowNodes)
	{
		$RowData = @{}
		foreach ($CellNode in $RowNode.SelectNodes('./a:c', $NamespaceManager))
		{
			$ReferenceAttribute = $CellNode.Attributes['r']
			$ColumnIndex = if ($null -ne $ReferenceAttribute) { Get-ColumnIndexFromReference -CellReference $ReferenceAttribute.Value } else { 0 }
			if ($ColumnIndex -le 0)
			{
				continue
			}

			$RowData[$ColumnIndex] = Convert-CellValue -Cell $CellNode -NamespaceManager $NamespaceManager -SharedStrings $SharedStrings
		}

		if ($RowData.Count -gt 0)
		{
			$ParsedRows.Add($RowData)
		}
	}

	if ($ParsedRows.Count -eq 0)
	{
		return @()
	}

	$HeaderRow = $ParsedRows[0]
	$HeaderColumns = $HeaderRow.Keys | Sort-Object
	$Headers = @{}
	foreach ($ColumnIndex in $HeaderColumns)
	{
		$HeaderName = [string]$HeaderRow[$ColumnIndex]
		if ([string]::IsNullOrWhiteSpace($HeaderName))
		{
			continue
		}

		$Headers[$ColumnIndex] = $HeaderName.Trim()
	}

	if ($Headers.Count -eq 0)
	{
		return @()
	}

	$HeaderColumnsToExport = $Headers.Keys | Sort-Object

	$ExportRows = New-Object System.Collections.Generic.List[object]
	for ($RowIndex = 1; $RowIndex -lt $ParsedRows.Count; $RowIndex++)
	{
		$ParsedRow = $ParsedRows[$RowIndex]
		$ExportRow = [ordered]@{}
		$HasValue = $false

		foreach ($HeaderColumn in $HeaderColumnsToExport)
		{
			$Value = if ($ParsedRow.ContainsKey($HeaderColumn)) { $ParsedRow[$HeaderColumn] } else { $null }
			if ($Value -is [string] -and [string]::IsNullOrWhiteSpace($Value))
			{
				$Value = $null
			}

			if ($null -ne $Value)
			{
				$HasValue = $true
			}

			$ExportRow[$Headers[$HeaderColumn]] = $Value
		}

		if ($HasValue)
		{
			$ExportRows.Add([pscustomobject]$ExportRow)
		}
	}

	return $ExportRows
}

New-Item -ItemType Directory -Path $TargetDir -Force | Out-Null

$WorkbookFiles = @(
if ($null -ne $WorkbookPaths -and $WorkbookPaths.Count -gt 0)
{
	foreach ($WorkbookPath in $WorkbookPaths)
	{
		if ([string]::IsNullOrWhiteSpace($WorkbookPath))
		{
			continue
		}

		$ResolvedPath = Resolve-Path -LiteralPath $WorkbookPath -ErrorAction Stop
		Get-Item -LiteralPath $ResolvedPath
	}
}
else
{
	Get-ChildItem -Path $SourceDir -File -Filter '*.xlsx' | Where-Object { -not $_.Name.StartsWith('~$') }
}
)

if ($WorkbookFiles.Count -eq 0)
{
	throw "No .xlsx files were found in '$SourceDir'."
}

foreach ($WorkbookFile in $WorkbookFiles)
{
	Write-Host "Exporting $($WorkbookFile.Name)..." -ForegroundColor Cyan

	$Archive = [System.IO.Compression.ZipFile]::OpenRead($WorkbookFile.FullName)
	try
	{
		$WorkbookText = Read-ZipEntryText -Archive $Archive -EntryPath 'xl/workbook.xml'
		$RelationshipsText = Read-ZipEntryText -Archive $Archive -EntryPath 'xl/_rels/workbook.xml.rels'

		if ([string]::IsNullOrWhiteSpace($WorkbookText) -or [string]::IsNullOrWhiteSpace($RelationshipsText))
		{
			throw "Workbook metadata is missing in '$($WorkbookFile.Name)'."
		}

		$WorkbookXml = [xml]$WorkbookText
		$RelationshipsXml = [xml]$RelationshipsText
		$SharedStrings = Get-SharedStringTable -Archive $Archive
		$WorksheetPath = Get-WorksheetTargetPath -WorkbookXml $WorkbookXml -RelationshipsXml $RelationshipsXml -WorksheetName $SheetName

		if ([string]::IsNullOrWhiteSpace($WorksheetPath))
		{
			Write-Warning "Skipped '$($WorkbookFile.Name)' because sheet '$SheetName' was not found."
			continue
		}

		$WorksheetText = Read-ZipEntryText -Archive $Archive -EntryPath $WorksheetPath
		if ([string]::IsNullOrWhiteSpace($WorksheetText))
		{
			Write-Warning "Skipped '$($WorkbookFile.Name)' because worksheet data could not be read."
			continue
		}

		$WorksheetXml = [xml]$WorksheetText
		$Rows = Convert-WorksheetToRows -WorksheetXml $WorksheetXml -SharedStrings $SharedStrings
		$JsonText = $Rows | ConvertTo-Json -Depth 10
		$OutputPath = Join-Path $TargetDir ($WorkbookFile.BaseName + '.json')
		[System.IO.File]::WriteAllText($OutputPath, $JsonText, [System.Text.UTF8Encoding]::new($false))

		Write-Host "  -> $OutputPath" -ForegroundColor Green
	}
	finally
	{
		$Archive.Dispose()
	}
}

Write-Host 'Export complete.' -ForegroundColor Green
