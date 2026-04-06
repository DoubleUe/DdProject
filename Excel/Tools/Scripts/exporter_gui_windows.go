//go:build windows

package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"sort"
	"strings"
	"sync"
	"syscall"
	"unsafe"
)

const (
	appTitle   = "Excel Table Exporter"
	windowClass = "DdProjectExcelExporterWindow"

	idListView       = 1001
	idRefreshButton  = 1002
	idExportButton   = 1003
	idExportAllButton = 1004

	wmCreate  = 0x0001
	wmDestroy = 0x0002
	wmSize    = 0x0005
	wmCommand = 0x0111
	wmSetFont = 0x0030
	wmApp     = 0x8000

	wmAppExportFinished = wmApp + 1

	wsOverlappedWindow = 0x00CF0000
	wsVisible          = 0x10000000
	wsChild            = 0x40000000
	wsTabStop          = 0x00010000
	wsBorder           = 0x00800000

	swShowDefault = 10

	cwUseDefault = uintptr(0x80000000)

	colorWindow = 5

	bsPushButton = 0x00000000

	lvsReport        = 0x0001
	lvsShowSelAlways = 0x0008
	lvsSingleSel     = 0x0004

	lvsExGridLines     = 0x00000001
	lvsExCheckboxes    = 0x00000004
	lvsExFullRowSelect = 0x00000020

	lvmFirst                    = 0x1000
	lvmDeleteAllItems           = lvmFirst + 9
	lvmGetItemCount             = lvmFirst + 4
	lvmGetItemState             = lvmFirst + 44
	lvmSetExtendedListViewStyle = lvmFirst + 54
	lvmInsertItemW              = lvmFirst + 77
	lvmInsertColumnW            = lvmFirst + 97
	lvmSetItemTextW             = lvmFirst + 116
	lvmSetColumnWidth           = lvmFirst + 30

	lvcfFmt   = 0x0001
	lvcfWidth = 0x0002
	lvcfText  = 0x0004

	lvifText = 0x0001

	lvisStateImageMask = 0xF000

	iccListViewClasses = 0x00000001

	defaultGuiFont = 17

	mbOK              = 0x00000000
	mbIconInformation = 0x00000040
	mbIconWarning     = 0x00000030
	mbIconError       = 0x00000010

	bnClicked = 0

	idcArrow = 32512
)

type wndClassEx struct {
	CbSize        uint32
	Style         uint32
	LpfnWndProc   uintptr
	CbClsExtra    int32
	CbWndExtra    int32
	HInstance     syscall.Handle
	HIcon         syscall.Handle
	HCursor       syscall.Handle
	HbrBackground syscall.Handle
	LpszMenuName  *uint16
	LpszClassName *uint16
	HIconSm       syscall.Handle
}

type msg struct {
	HWnd    syscall.Handle
	Message uint32
	WParam  uintptr
	LParam  uintptr
	Time    uint32
	Pt      point
}

type point struct {
	X int32
	Y int32
}

type rect struct {
	Left   int32
	Top    int32
	Right  int32
	Bottom int32
}

type initCommonControlsEx struct {
	DwSize uint32
	DwICC  uint32
}

type lvColumn struct {
	Mask       uint32
	Fmt        int32
	Cx         int32
	PszText    *uint16
	CchTextMax int32
	ISubItem   int32
	IImage     int32
	IOrder     int32
	CxMin      int32
	CxDefault  int32
	CxIdeal    int32
}

type lvItem struct {
	Mask       uint32
	IItem      int32
	ISubItem   int32
	State      uint32
	StateMask  uint32
	PszText    *uint16
	CchTextMax int32
	IImage     int32
	LParam     uintptr
	IIndent    int32
	IGroupId   int32
	CColumns   uint32
	PuColumns  *uint32
	PiColFmt   *int32
	IGroup     int32
}

type excelFile struct {
	Name       string
	FullPath   string
	OutputName string
}

type appState struct {
	hwnd           syscall.Handle
	hListView      syscall.Handle
	hRefreshButton syscall.Handle
	hExportButton  syscall.Handle
	hExportAllButton syscall.Handle
	hSourceLabel   syscall.Handle
	hTargetLabel   syscall.Handle
	hHintLabel     syscall.Handle
	font           syscall.Handle
	files          []excelFile
	isExporting    bool
}

type exportResult struct {
	output        string
	err           error
	exportedCount int
}

var (
	user32   = syscall.NewLazyDLL("user32.dll")
	kernel32 = syscall.NewLazyDLL("kernel32.dll")
	gdi32    = syscall.NewLazyDLL("gdi32.dll")
	comctl32 = syscall.NewLazyDLL("comctl32.dll")

	procCreateWindowExW   = user32.NewProc("CreateWindowExW")
	procDefWindowProcW    = user32.NewProc("DefWindowProcW")
	procDispatchMessageW  = user32.NewProc("DispatchMessageW")
	procEnableWindow      = user32.NewProc("EnableWindow")
	procGetClientRect     = user32.NewProc("GetClientRect")
	procGetMessageW       = user32.NewProc("GetMessageW")
	procLoadCursorW       = user32.NewProc("LoadCursorW")
	procMessageBoxW       = user32.NewProc("MessageBoxW")
	procMoveWindow        = user32.NewProc("MoveWindow")
	procPostMessageW      = user32.NewProc("PostMessageW")
	procPostQuitMessage   = user32.NewProc("PostQuitMessage")
	procRegisterClassExW  = user32.NewProc("RegisterClassExW")
	procSendMessageW      = user32.NewProc("SendMessageW")
	procShowWindow        = user32.NewProc("ShowWindow")
	procTranslateMessage  = user32.NewProc("TranslateMessage")
	procUpdateWindow      = user32.NewProc("UpdateWindow")

	procGetModuleHandleW = kernel32.NewProc("GetModuleHandleW")
	procGetStockObject   = gdi32.NewProc("GetStockObject")
	procInitCommonControlsEx = comctl32.NewProc("InitCommonControlsEx")

	mainWindowProc = syscall.NewCallback(windowProc)
	state          = &appState{}
	exportResultMu sync.Mutex
	lastExportResult *exportResult
)

func main() {
	runtime.LockOSThread()

	initCommonControls()

	hInstance, err := getModuleHandle()
	if err != nil {
		showMessage(0, appTitle, fmt.Sprintf("Failed to get module handle.\n\n%v", err), mbIconError)
		return
	}

	if err := registerWindowClass(hInstance); err != nil {
		showMessage(0, appTitle, fmt.Sprintf("Failed to register window class.\n\n%v", err), mbIconError)
		return
	}

	hwnd, err := createMainWindow(hInstance)
	if err != nil {
		showMessage(0, appTitle, fmt.Sprintf("Failed to create window.\n\n%v", err), mbIconError)
		return
	}

	state.hwnd = hwnd
	showWindow(hwnd)
	runMessageLoop()
}

func initCommonControls() {
	icc := initCommonControlsEx{
		DwSize: uint32(unsafe.Sizeof(initCommonControlsEx{})),
		DwICC:  iccListViewClasses,
	}
	procInitCommonControlsEx.Call(uintptr(unsafe.Pointer(&icc)))
}

func getModuleHandle() (syscall.Handle, error) {
	ret, _, err := procGetModuleHandleW.Call(0)
	if ret == 0 {
		return 0, err
	}

	return syscall.Handle(ret), nil
}

func registerWindowClass(hInstance syscall.Handle) error {
	className, _ := syscall.UTF16PtrFromString(windowClass)

	cursor, _, _ := procLoadCursorW.Call(0, uintptr(idcArrow))
	wc := wndClassEx{
		CbSize:        uint32(unsafe.Sizeof(wndClassEx{})),
		LpfnWndProc:   mainWindowProc,
		HInstance:     hInstance,
		HCursor:       syscall.Handle(cursor),
		HbrBackground: syscall.Handle(colorWindow + 1),
		LpszClassName: className,
	}

	ret, _, err := procRegisterClassExW.Call(uintptr(unsafe.Pointer(&wc)))
	if ret == 0 {
		return err
	}

	return nil
}

func createMainWindow(hInstance syscall.Handle) (syscall.Handle, error) {
	className, _ := syscall.UTF16PtrFromString(windowClass)
	title, _ := syscall.UTF16PtrFromString(appTitle)

	ret, _, err := procCreateWindowExW.Call(
		0,
		uintptr(unsafe.Pointer(className)),
		uintptr(unsafe.Pointer(title)),
		uintptr(wsOverlappedWindow|wsVisible),
		cwUseDefault,
		cwUseDefault,
		820,
		560,
		0,
		0,
		uintptr(hInstance),
		0,
	)
	if ret == 0 {
		return 0, err
	}

	return syscall.Handle(ret), nil
}

func runMessageLoop() {
	var message msg
	for {
		ret, _, _ := procGetMessageW.Call(uintptr(unsafe.Pointer(&message)), 0, 0, 0)
		if int32(ret) <= 0 {
			return
		}

		procTranslateMessage.Call(uintptr(unsafe.Pointer(&message)))
		procDispatchMessageW.Call(uintptr(unsafe.Pointer(&message)))
	}
}

func windowProc(hwnd, message, wParam, lParam uintptr) uintptr {
	windowHandle := syscall.Handle(hwnd)

	switch uint32(message) {
	case wmCreate:
		state.hwnd = windowHandle
		state.font = getDefaultFont()
		createChildControls(windowHandle)
		refreshFileList()
		layoutControls(windowHandle)
		return 0
	case wmCommand:
		handleCommand(windowHandle, wParam)
		return 0
	case wmSize:
		layoutControls(windowHandle)
		return 0
	case wmAppExportFinished:
		handleExportFinished(windowHandle)
		return 0
	case wmDestroy:
		procPostQuitMessage.Call(0)
		return 0
	default:
		ret, _, _ := procDefWindowProcW.Call(hwnd, message, wParam, lParam)
		return ret
	}
}

func createChildControls(hwnd syscall.Handle) {
	state.hSourceLabel = createControl("STATIC", "", wsChild|wsVisible, 0, hwnd, 0)
	state.hTargetLabel = createControl("STATIC", "", wsChild|wsVisible, 0, hwnd, 0)
	state.hHintLabel = createControl("STATIC", "Check files to export selected items, or use Export All.", wsChild|wsVisible, 0, hwnd, 0)

	listStyle := uint32(wsChild | wsVisible | wsTabStop | wsBorder | lvsReport | lvsShowSelAlways | lvsSingleSel)
	state.hListView = createControl("SysListView32", "", listStyle, 0, hwnd, idListView)

	state.hRefreshButton = createControl("BUTTON", "Refresh", wsChild|wsVisible|wsTabStop|bsPushButton, 0, hwnd, idRefreshButton)
	state.hExportButton = createControl("BUTTON", "Export Selected", wsChild|wsVisible|wsTabStop|bsPushButton, 0, hwnd, idExportButton)
	state.hExportAllButton = createControl("BUTTON", "Export All", wsChild|wsVisible|wsTabStop|bsPushButton, 0, hwnd, idExportAllButton)

	applyFont(state.hSourceLabel)
	applyFont(state.hTargetLabel)
	applyFont(state.hHintLabel)
	applyFont(state.hListView)
	applyFont(state.hRefreshButton)
	applyFont(state.hExportButton)
	applyFont(state.hExportAllButton)

	initListViewColumns()
}

func createControl(className, text string, style uint32, exStyle uint32, parent syscall.Handle, id int) syscall.Handle {
	classNamePtr, _ := syscall.UTF16PtrFromString(className)
	textPtr, _ := syscall.UTF16PtrFromString(text)

	ret, _, _ := procCreateWindowExW.Call(
		uintptr(exStyle),
		uintptr(unsafe.Pointer(classNamePtr)),
		uintptr(unsafe.Pointer(textPtr)),
		uintptr(style),
		0,
		0,
		100,
		30,
		uintptr(parent),
		uintptr(id),
		0,
		0,
	)

	return syscall.Handle(ret)
}

func getDefaultFont() syscall.Handle {
	ret, _, _ := procGetStockObject.Call(defaultGuiFont)
	return syscall.Handle(ret)
}

func applyFont(hwnd syscall.Handle) {
	if hwnd == 0 || state.font == 0 {
		return
	}

	procSendMessageW.Call(uintptr(hwnd), wmSetFont, uintptr(state.font), 1)
}

func initListViewColumns() {
	extendedStyle := uintptr(lvsExGridLines | lvsExCheckboxes | lvsExFullRowSelect)
	procSendMessageW.Call(uintptr(state.hListView), lvmSetExtendedListViewStyle, extendedStyle, extendedStyle)

	insertListViewColumn(0, "Excel File", 240)
	insertListViewColumn(1, "Sheet", 120)
	insertListViewColumn(2, "Output JSON", 220)
}

func insertListViewColumn(index int32, title string, width int32) {
	titlePtr, _ := syscall.UTF16PtrFromString(title)
	column := lvColumn{
		Mask:    lvcfText | lvcfWidth | lvcfFmt,
		Fmt:     0,
		Cx:      width,
		PszText: titlePtr,
	}

	procSendMessageW.Call(uintptr(state.hListView), lvmInsertColumnW, uintptr(index), uintptr(unsafe.Pointer(&column)))
}

func refreshFileList() {
	files, err := findExcelFiles()
	if err != nil {
		showMessage(state.hwnd, appTitle, fmt.Sprintf("Failed to scan Excel files.\n\n%v", err), mbIconError)
		return
	}

	state.files = files
	updateLabels()
	reloadListView()
}

func updateLabels() {
	excelDir, _ := excelDirectory()
	projectRoot := filepath.Dir(excelDir)
	targetDir := filepath.Join(projectRoot, "Content", "Json", "Table")

	setWindowText(state.hSourceLabel, "Source: "+excelDir)
	setWindowText(state.hTargetLabel, "Target: "+targetDir)
}

func reloadListView() {
	procSendMessageW.Call(uintptr(state.hListView), lvmDeleteAllItems, 0, 0)

	for index, file := range state.files {
		insertListViewItem(int32(index), file.Name)
		setListViewSubItem(int32(index), 1, "GameData")
		setListViewSubItem(int32(index), 2, file.OutputName)
	}

	resizeListViewColumns()
}

func insertListViewItem(index int32, text string) {
	textPtr, _ := syscall.UTF16PtrFromString(text)
	item := lvItem{
		Mask:    lvifText,
		IItem:   index,
		ISubItem: 0,
		PszText: textPtr,
	}

	procSendMessageW.Call(uintptr(state.hListView), lvmInsertItemW, 0, uintptr(unsafe.Pointer(&item)))
}

func setListViewSubItem(index int32, subItem int32, text string) {
	textPtr, _ := syscall.UTF16PtrFromString(text)
	item := lvItem{
		ISubItem: subItem,
		PszText:  textPtr,
	}

	procSendMessageW.Call(uintptr(state.hListView), lvmSetItemTextW, uintptr(index), uintptr(unsafe.Pointer(&item)))
}

func handleCommand(hwnd syscall.Handle, wParam uintptr) {
	commandID := lowWord(wParam)
	notificationCode := highWord(wParam)
	if notificationCode != bnClicked {
		return
	}

	switch commandID {
	case idRefreshButton:
		refreshFileList()
	case idExportButton:
		selectedPaths := getCheckedWorkbookPaths()
		if len(selectedPaths) == 0 {
			showMessage(hwnd, appTitle, "No Excel file is checked.", mbIconWarning)
			return
		}

		runExport(hwnd, selectedPaths, false)
	case idExportAllButton:
		if len(state.files) == 0 {
			showMessage(hwnd, appTitle, "No Excel files were found to export.", mbIconWarning)
			return
		}

		runExport(hwnd, nil, true)
	}
}

func runExport(hwnd syscall.Handle, selectedPaths []string, exportAll bool) {
	if state.isExporting {
		return
	}

	state.isExporting = true
	setButtonsEnabled(false)

	exportedCount := len(selectedPaths)
	if exportAll {
		exportedCount = len(state.files)
	}

	if exportedCount <= 0 {
		exportedCount = len(state.files)
	}

	go func(window syscall.Handle, workbookPaths []string, count int) {
		output, err := executeExporter(workbookPaths)
		setLastExportResult(&exportResult{
			output:        output,
			err:           err,
			exportedCount: count,
		})
		procPostMessageW.Call(uintptr(window), wmAppExportFinished, 0, 0)
	}(hwnd, append([]string(nil), selectedPaths...), exportedCount)
}

func handleExportFinished(hwnd syscall.Handle) {
	result := takeLastExportResult()
	state.isExporting = false
	setButtonsEnabled(true)

	if result == nil {
		showMessage(hwnd, appTitle, "Export finished, but no result payload was available.", mbIconWarning)
		return
	}

	if result.err != nil {
		showMessage(hwnd, appTitle, fmt.Sprintf("Export failed.\n\n%s", trimOutputForDialog(result.err.Error())), mbIconError)
		return
	}

	successMessage := fmt.Sprintf("%d file(s) exported successfully.", result.exportedCount)
	if strings.TrimSpace(result.output) != "" {
		successMessage += "\n\n" + trimOutputForDialog(result.output)
	}

	showMessage(hwnd, appTitle, successMessage, mbIconInformation)
}

func setLastExportResult(result *exportResult) {
	exportResultMu.Lock()
	defer exportResultMu.Unlock()
	lastExportResult = result
}

func takeLastExportResult() *exportResult {
	exportResultMu.Lock()
	defer exportResultMu.Unlock()

	result := lastExportResult
	lastExportResult = nil
	return result
}

func executeExporter(selectedPaths []string) (string, error) {
	toolsDir, err := executableDirectory()
	if err != nil {
		return "", err
	}

	excelDir := filepath.Dir(toolsDir)
	projectRoot := filepath.Dir(excelDir)
	scriptPath := filepath.Join(toolsDir, "Scripts", "ExportExcelTablesToJson.ps1")
	targetDir := filepath.Join(projectRoot, "Content", "Json", "Table")

	args := []string{
		"-NoProfile",
		"-ExecutionPolicy", "Bypass",
		"-File", scriptPath,
		"-SourceDir", excelDir,
		"-TargetDir", targetDir,
	}

	if len(selectedPaths) > 0 {
		args = append(args, "-WorkbookPaths")
		args = append(args, selectedPaths...)
	}

	cmd := exec.Command("powershell.exe", args...)
	cmd.SysProcAttr = &syscall.SysProcAttr{HideWindow: true}
	output, err := cmd.CombinedOutput()
	if err != nil {
		if len(output) == 0 {
			return "", err
		}

		return string(output), fmt.Errorf("%v\n\n%s", err, string(output))
	}

	return string(output), nil
}

func getCheckedWorkbookPaths() []string {
	count, _, _ := procSendMessageW.Call(uintptr(state.hListView), lvmGetItemCount, 0, 0)
	checkedPaths := make([]string, 0)

	for i := 0; i < int(count); i++ {
		stateValue, _, _ := procSendMessageW.Call(
			uintptr(state.hListView),
			lvmGetItemState,
			uintptr(i),
			uintptr(lvisStateImageMask),
		)

		if ((stateValue >> 12) & 0xF) == 2 && i < len(state.files) {
			checkedPaths = append(checkedPaths, state.files[i].FullPath)
		}
	}

	return checkedPaths
}

func setButtonsEnabled(enabled bool) {
	value := uintptr(0)
	if enabled {
		value = 1
	}

	procEnableWindow.Call(uintptr(state.hRefreshButton), value)
	procEnableWindow.Call(uintptr(state.hExportButton), value)
	procEnableWindow.Call(uintptr(state.hExportAllButton), value)
}

func findExcelFiles() ([]excelFile, error) {
	excelDir, err := excelDirectory()
	if err != nil {
		return nil, err
	}
	entries, err := os.ReadDir(excelDir)
	if err != nil {
		return nil, err
	}

	files := make([]excelFile, 0)
	for _, entry := range entries {
		if entry.IsDir() {
			continue
		}

		name := entry.Name()
		lowerName := strings.ToLower(name)
		if !strings.HasSuffix(lowerName, ".xlsx") || strings.HasPrefix(name, "~$") {
			continue
		}

		baseName := strings.TrimSuffix(name, filepath.Ext(name))
		files = append(files, excelFile{
			Name:       name,
			FullPath:   filepath.Join(excelDir, name),
			OutputName: baseName + ".json",
		})
	}

	sort.Slice(files, func(i, j int) bool {
		return strings.ToLower(files[i].Name) < strings.ToLower(files[j].Name)
	})

	return files, nil
}

func executableDirectory() (string, error) {
	executablePath, err := os.Executable()
	if err != nil {
		return "", err
	}

	return filepath.Dir(executablePath), nil
}

func excelDirectory() (string, error) {
	toolsDir, err := executableDirectory()
	if err != nil {
		return "", err
	}

	return filepath.Dir(toolsDir), nil
}

func layoutControls(hwnd syscall.Handle) {
	if hwnd == 0 || state.hListView == 0 {
		return
	}

	clientRect, ok := getClientRect(hwnd)
	if !ok {
		return
	}

	margin := int32(12)
	labelHeight := int32(20)
	hintHeight := int32(18)
	buttonHeight := int32(32)
	buttonWidth := int32(120)
	buttonGap := int32(10)

	clientWidth := clientRect.Right - clientRect.Left
	clientHeight := clientRect.Bottom - clientRect.Top

	top := margin
	moveWindow(state.hSourceLabel, margin, top, clientWidth-(margin*2), labelHeight)
	top += labelHeight + 4

	moveWindow(state.hTargetLabel, margin, top, clientWidth-(margin*2), labelHeight)
	top += labelHeight + 6

	moveWindow(state.hHintLabel, margin, top, clientWidth-(margin*2), hintHeight)
	top += hintHeight + 10

	buttonTop := clientHeight - margin - buttonHeight
	listHeight := buttonTop - top - 10
	if listHeight < 120 {
		listHeight = 120
	}

	moveWindow(state.hListView, margin, top, clientWidth-(margin*2), listHeight)

	right := clientWidth - margin
	exportAllLeft := right - buttonWidth
	exportLeft := exportAllLeft - buttonGap - 140
	refreshLeft := exportLeft - buttonGap - buttonWidth

	moveWindow(state.hRefreshButton, refreshLeft, buttonTop, buttonWidth, buttonHeight)
	moveWindow(state.hExportButton, exportLeft, buttonTop, 140, buttonHeight)
	moveWindow(state.hExportAllButton, exportAllLeft, buttonTop, buttonWidth, buttonHeight)

	resizeListViewColumns()
}

func resizeListViewColumns() {
	if state.hListView == 0 {
		return
	}

	clientRect, ok := getClientRect(state.hListView)
	if !ok {
		return
	}

	width := clientRect.Right - clientRect.Left
	if width <= 0 {
		return
	}

	fileWidth := int32(float64(width) * 0.42)
	sheetWidth := int32(float64(width) * 0.18)
	outputWidth := width - fileWidth - sheetWidth - 4

	setListViewColumnWidth(0, fileWidth)
	setListViewColumnWidth(1, sheetWidth)
	setListViewColumnWidth(2, outputWidth)
}

func setListViewColumnWidth(index int32, width int32) {
	procSendMessageW.Call(uintptr(state.hListView), lvmSetColumnWidth, uintptr(index), uintptr(width))
}

func getClientRect(hwnd syscall.Handle) (rect, bool) {
	var rc rect
	ret, _, _ := procGetClientRect.Call(uintptr(hwnd), uintptr(unsafe.Pointer(&rc)))
	return rc, ret != 0
}

func moveWindow(hwnd syscall.Handle, x, y, width, height int32) {
	procMoveWindow.Call(uintptr(hwnd), uintptr(x), uintptr(y), uintptr(width), uintptr(height), 1)
}

func lowWord(value uintptr) int {
	return int(uint16(value & 0xFFFF))
}

func highWord(value uintptr) int {
	return int(uint16((value >> 16) & 0xFFFF))
}

func setWindowText(hwnd syscall.Handle, text string) {
	textPtr, _ := syscall.UTF16PtrFromString(text)
	procSendMessageW.Call(uintptr(hwnd), 0x000C, 0, uintptr(unsafe.Pointer(textPtr)))
}

func showWindow(hwnd syscall.Handle) {
	procShowWindow.Call(uintptr(hwnd), swShowDefault)
	procUpdateWindow.Call(uintptr(hwnd))
}

func showMessage(hwnd syscall.Handle, title, text string, style uintptr) {
	titlePtr, _ := syscall.UTF16PtrFromString(title)
	textPtr, _ := syscall.UTF16PtrFromString(text)
	procMessageBoxW.Call(uintptr(hwnd), uintptr(unsafe.Pointer(textPtr)), uintptr(unsafe.Pointer(titlePtr)), mbOK|style)
}

func trimOutputForDialog(text string) string {
	trimmed := strings.TrimSpace(text)
	if trimmed == "" {
		return ""
	}

	const maxLen = 1800
	if len(trimmed) <= maxLen {
		return trimmed
	}

	return trimmed[:maxLen] + "\n\n...(truncated)"
}
