
add_executable(pdfquirk
    main.cpp
    dialog.cpp
    imagemodel.cpp
    imagemodel.h
    imagelistdelegate.cpp
    imagelistdelegate.h
    pdfcreator.cpp
    pdfcreator.h
    dialog.h
    dialog.ui
    ../resources/pdfquirk.qrc
)

target_link_libraries(pdfquirk PRIVATE Qt5::Widgets)


