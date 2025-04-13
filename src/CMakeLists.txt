set(SOURCES
    main.cpp
    dialog.cpp
    imagemodel.cpp
    imagemodel.h
    imagelistdelegate.cpp
    pdfquirkimage.cpp
    settings.cpp
    pdfexporter.cpp
    settings.h
    imagelistdelegate.h
    executor.cpp
    executor.h
    version.h
    dialog.h
    dialog.ui
)

add_executable(pdfquirk
    ${SOURCES} ../resources/pdfquirk.qrc
)

set ( Translations ../resources/pdfquirk_de.ts ../resources/pdfquirk_es.ts)
set_source_files_properties(${Translations} PROPERTIES OUTPUT_LOCATION
${CMAKE_SOURCE_DIR}/resources)

if(Qt6_Found)
    qt6_create_translation(QM_FILES ${SOURCES} ${Translations})
    qt6_add_translations(pdfquirk TS_FILES ${Translations})
    target_link_libraries(pdfquirk PRIVATE Qt6::Widgets)
    message ("Linking against Qt6!")
else(Qt6_Found)
    qt5_create_translation(QM_FILES ${SOURCES} ${Translations})
    qt5_add_translation(pdfquirk TS_FILES ${Translations})
    target_link_libraries(pdfquirk PRIVATE Qt5::Widgets)
    message ("Linking against Qt5!")
endif(Qt6_Found)

install(TARGETS pdfquirk RUNTIME DESTINATION bin)
