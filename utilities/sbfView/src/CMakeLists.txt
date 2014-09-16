file(GLOB SRC_LIST_loc
sbfviewmainwindow.cpp
sbfview.cpp
sbfmodel.cpp
sbfdataitem.cpp
sbfdatamodel.cpp
sbftoolbar.cpp
)
file(GLOB UI_LIST_loc
)
file(GLOB MOC_LIST_loc
sbfviewmainwindow.h
sbfview.h
sbfmodel.h
sbfdataitem.h
sbfdatamodel.h
sbftoolbar.h
)

set(SRC_LIST ${SRC_LIST} ${SRC_LIST_loc} PARENT_SCOPE)
set(UI_LIST ${UI_LIST}   ${UI_LIST_loc} PARENT_SCOPE)
set(MOC_LIST ${MOC_LIST} ${MOC_LIST_loc} PARENT_SCOPE)
