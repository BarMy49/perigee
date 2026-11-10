THEME_SOURCE := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
THEME_BUILD := $(abspath ../theme-tests)
THEME_OBJECTS := $(filter-out debug/main.o,$(OBJECTS))
THEME_TEST_FLAGS := $(shell pkg-config --cflags Qt6Test)
THEME_TEST_LIBS := $(shell pkg-config --libs Qt6Test)

.PHONY: theme-check-build
theme-check-build: $(THEME_BUILD)/tst_uitheme

$(THEME_BUILD):
	mkdir -p $@

$(THEME_BUILD)/tst_uitheme.moc: $(THEME_SOURCE)/tst_uitheme.cpp | $(THEME_BUILD)
	/usr/lib64/qt6/libexec/moc $(DEFINES) $(INCPATH) $(THEME_TEST_FLAGS) $< -o $@

$(THEME_BUILD)/tst_uitheme.o: $(THEME_SOURCE)/tst_uitheme.cpp $(THEME_BUILD)/tst_uitheme.moc
	$(CXX) $(CXXFLAGS) $(INCPATH) $(THEME_TEST_FLAGS) -I$(THEME_BUILD) -c $< -o $@

$(THEME_BUILD)/tst_uitheme: $(THEME_BUILD)/tst_uitheme.o $(THEME_OBJECTS)
	$(LINK) $(LFLAGS) -o $@ $^ $(LIBS) $(THEME_TEST_LIBS)
