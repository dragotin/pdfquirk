

add_executable(pdfquirk
    main.cpp
    dialog.cpp
    imagemodel.cpp
    imagemodel.h
    imagelistdelegate.cpp
    pdfquirkimage.cpp
    imagelistdelegate.h
    executor.cpp
    executor.h
    version.h
    dialog.h
    dialog.ui
    ../resources/pdfquirk.qrc
)

qt6_add_translations(pdfquirk TS_FILES  pdfquirk_de.ts)

if(Qt6_Found)
    target_link_libraries(pdfquirk PRIVATE Qt6::Widgets)
    message ("Linking against Qt6!")
else(Qt6_Found)
    target_link_libraries(pdfquirk PRIVATE Qt5::Widgets)
    message ("Linking against Qt5!")
endif(Qt6_Found)

install(TARGETS pdfquirk RUNTIME DESTINATION bin)
