SOURCES = $(wildcard *.gresource.xml)
OBJECTS = $(SOURCES:.gresource.xml=.gresource)


%.gresource: %.gresource.xml
	glib-compile-resources $<


all: $(OBJECTS)

