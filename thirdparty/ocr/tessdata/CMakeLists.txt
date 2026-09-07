buildmaster_component(
	tessdata
	"Tesseract traineddata"
	"${CMAKE_CURRENT_LIST_DIR}/cmake-wrapper"
	""
	static
	"tessdata"
)

buildmaster_link(StormByte-Multimedia tessdata)
buildmaster_group_add(OCR tessdata)