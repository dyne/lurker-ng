opt_cxx      = c++
opt_cppflags = 
opt_cxxflags = -Wall -O2
opt_picflags = -fpic -fPIC -DPIC
opt_soflags  = -shared -Wl,-soname=${soname}

tests   := test_member_template.cpp test_partial_specialization.cpp \
           test_std_namespace.cpp test_template_templates.cpp \
           test_operator_namespace.cpp
targets := $(patsubst %.cpp,$D/%.o,$(tests))

$D/config:	$(targets)
	@echo The compiler `cat .opt/cxx` appears to work.

$D/distclean:
	rm -f $(targets)

.%.d:	%.cpp $(opt_cxx) $(opt_cppflags) .%.opts
	@echo $(opts) '-M $< >roughly> $@'
	@$(opts) -M $< > $@.tmp
	@sed 's@$(*F)\.o[ :]*@$*.o $*.lo $@ : @g' <$@.tmp >$@
	@rm -f $@.tmp

# Now for our rules
%.o:	%.cpp .%.d $(opt_cxx) $(opt_cppflags) $(opt_cxxflags) .%.opts
	$(opts) -c $< -o $@

%.lo:	%.cpp .%.d $(opt_cxx) $(opt_cppflags) $(opt_cxxflags) $(opt_picflags) .%.opts
	$(opts) -c $< -o $@

la_version=$(shell cat $(filter %.version, $^))
la_major=$(shell echo $(la_version) | sed 's@\([^. ]*\).*$$@\1@') 
soname=$(*F).so.$(la_major)

%.la:	$(opt_cxx) $(opt_soflags) %.version .%.link.opts
	soname=$(soname); $(opts) $(filter-out %.version, $(deps)) \
		-o $*.so.$(la_version)
	ln -fs $(*F).so.$(la_version) $*.so.$(la_major)
	ln -fs $(*F).so.$(la_version) $*.so
	touch $@

%:	%.o $(opt_cxx) .%.link.opts
	$(opts) $(deps) -o $@
