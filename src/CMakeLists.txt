

add_executable(pdfquirk
    main.cpp
    dialog.cpp
    imagemodel.cpp
    imagemodel.h
    imagelistdelegate.cpp
    pdfquirkimage.cpp
    settings.cpp
    settings.h
    imagelistdelegate.h
    executor.cpp
    executor.h
    version.h
    dialog.h
    dialog.ui
    ../resources/pdfquirk.qrc
)

set ( Translations ../resources/pdfquirk_de.ts )
set_source_files_properties(${Translations} PROPERTIES OUTPUT_LOCATION
${CMAKE_CURRENT_SOURCE_DIR}/../resources)

if(Qt6_Found)
    qt6_add_translations(pdfquirk TS_FILES ${Translations})
    target_link_libraries(pdfquirk PRIVATE Qt6::Widgets)
    message ("Linking against Qt6!")
else(Qt6_Found)
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${Translations})
    qt5_add_translation(pdfquirk TS_FILES ${Translations})
    target_link_libraries(pdfquirk PRIVATE Qt5::Widgets)
    message ("Linking against Qt5!")
endif(Qt6_Found)

install(TARGETS pdfquirk RUNTIME DESTINATION bin)
