
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

target_link_libraries(pdfquirk PRIVATE Qt::Widgets Qt::Core)

install(TARGETS pdfquirk RUNTIME DESTINATION bin)

