FAVORITES_SOURCE := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
FAVORITES_BUILD := $(abspath ../favorites-tests)
FAVORITES_OBJECTS := $(filter-out debug/main.o,$(OBJECTS))
FAVORITES_TEST_FLAGS := $(shell pkg-config --cflags Qt6Test)
FAVORITES_TEST_LIBS := $(shell pkg-config --libs Qt6Test)

.PHONY: favorites-check-build
favorites-check-build: $(FAVORITES_BUILD)/tst_appfavorites

$(FAVORITES_BUILD):
	mkdir -p $@

$(FAVORITES_BUILD)/tst_appfavorites.moc: $(FAVORITES_SOURCE)/tst_appfavorites.cpp | $(FAVORITES_BUILD)
	/usr/lib64/qt6/libexec/moc $(DEFINES) $(INCPATH) $(FAVORITES_TEST_FLAGS) $< -o $@

$(FAVORITES_BUILD)/tst_appfavorites.o: $(FAVORITES_SOURCE)/tst_appfavorites.cpp $(FAVORITES_BUILD)/tst_appfavorites.moc
	$(CXX) $(CXXFLAGS) $(INCPATH) $(FAVORITES_TEST_FLAGS) -I$(FAVORITES_BUILD) -c $< -o $@

$(FAVORITES_BUILD)/tst_appfavorites: $(FAVORITES_BUILD)/tst_appfavorites.o $(FAVORITES_OBJECTS)
	$(LINK) $(LFLAGS) -o $@ $^ $(LIBS) $(FAVORITES_TEST_LIBS)
