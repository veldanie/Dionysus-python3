include $(CGAL_MAKEFILE)

all: libpath libpaths ldflags cxxflags

libpath:
	@echo "$(CGAL_LIB_DIR)/$(CGAL_OS_COMPILER)"

libpaths:
	@echo "$(CGAL_LIBPATHFLAGS)"

ldflags:
	@echo "$(CGAL_LDFLAGS)"

cxxflags:
	@echo "$(CGAL_CXXFLAGS)"
