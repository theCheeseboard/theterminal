TEMPLATE = subdirs

SUBDIRS += \
    terminal


blueprint {
    message(Configuring theTerminal to be built as blueprint)
    DEFINES += "BLUEPRINT"
} else {
    message(Configuring theTerminal to be built as stable)
}
