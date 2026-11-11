THEME_SOURCE := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
THEME_BUILD := $(abspath ../theme-tests)
THEME_OBJECTS := $(filter-out debug/main.o release/main.o,$(OBJECTS))
THEME_MOC := $(shell $(QMAKE) -query QT_HOST_LIBEXECS)/moc
THEME_TEST_FLAGS := -I$(shell $(QMAKE) -query QT_INSTALL_HEADERS)/QtTest
THEME_TEST_LIBS := -L$(shell $(QMAKE) -query QT_INSTALL_LIBS) -lQt6Test

.PHONY: theme-check-build
theme-check-build: $(THEME_BUILD)/tst_uitheme

$(THEME_BUILD):
	mkdir -p $@

$(THEME_BUILD)/tst_uitheme.moc: $(THEME_SOURCE)/tst_uitheme.cpp | $(THEME_BUILD)
	$(THEME_MOC) $(DEFINES) $(INCPATH) $(THEME_TEST_FLAGS) $< -o $@

$(THEME_BUILD)/tst_uitheme.o: $(THEME_SOURCE)/tst_uitheme.cpp $(THEME_BUILD)/tst_uitheme.moc
	$(CXX) $(CXXFLAGS) $(INCPATH) $(THEME_TEST_FLAGS) -I$(THEME_BUILD) -c $< -o $@

$(THEME_BUILD)/tst_uitheme: $(THEME_BUILD)/tst_uitheme.o $(THEME_OBJECTS)
	$(LINK) $(LFLAGS) -o $@ $^ $(LIBS) $(THEME_TEST_LIBS)
