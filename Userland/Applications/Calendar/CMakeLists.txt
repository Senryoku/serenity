serenity_component(
    Calendar
    TARGETS Calendar
)

stringify_gml(CalendarWindow.gml CalendarWindowGML.h calendar_window_gml)
stringify_gml(AddEventDialog.gml AddEventDialogGML.h add_event_dialog_gml)

set(SOURCES
    AddEventDialog.cpp
    CalendarWidget.cpp
    EventCalendar.cpp
    EventManager.cpp
    main.cpp
)

set(GENERATED_SOURCES
    CalendarWindowGML.h AddEventDialogGML.h
)

serenity_app(Calendar ICON app-calendar)
target_link_libraries(Calendar PRIVATE LibConfig LibCore LibFileSystem LibFileSystemAccessClient LibGfx LibGUI LibMain)
