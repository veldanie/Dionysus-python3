import os

# Get options
options_filenames =		['config/' + os.uname()[1] + '.py', os.path.expanduser('~/.scons/config.py')]

for fn in options_filenames:
	if os.path.exists(fn):
		optns =			Options(fn)
		print "Found configuration:", fn
		break
else:
		optns =			Options()

optns.AddOptions		(PathOption('cgal_path', 'where CGAL is installed', ''),
						('cgal_architecture', 'string describing CGAL architecture'),
						('INCPATH', 'additional include paths separated with :',''),
						('LIBPATH', 'additional library paths separated with :',''),
						PathOption('qt4_includes', 'where Qt4 headers are installed', ''),
						PathOption('qt4_libs', 'where Qt4 libraries are installed', ''),
						('CXX', 'The C++ compiler'))

# Setup environment
base_env = 				Environment(options = optns,
									ENV = os.environ,
									tools=["default", "doxygen"], #, "qt4"], 
									toolpath=["sconstools"])
Help					(optns.GenerateHelpText(base_env))

dbg =					base_env.Copy(CPPFLAGS = ['-g'])
opt = 					base_env.Copy(CPPDEFINES = ['NDEBUG'])

if ARGUMENTS.get('debug', 0):
	env = 				dbg
	env.Append			(CPPDEFINES = ['CWDEBUG'])
	env.Append			(LIBS = ['cwd'])
elif ARGUMENTS.get('optimize', 0):
	opt.Append 			(CPPFLAGS = ['-O' + str(ARGUMENTS['optimize'])])
	env =				opt
else:
	opt.Append 			(CPPFLAGS = ['-O'])
	env =				opt

if ARGUMENTS.get('counters', 0):
	env.Append			(CPPDEFINES = ['COUNTERS'])

# Don't create .sconsign files all over the place
SConsignFile()

# Create tags file
#tags =					env.Command('tags', 'sources',			# FIXME
#									['ctags '])

# CGAL
cgal_path = 			env['cgal_path']
cgal_architecture = 	env['cgal_architecture']	

cgal_defines = 			['CGAL_USE_GMP', 'CGAL_USE_GMPXX', 'CGAL_USE_CORE']
cgal_cpppath =			[cgal_path + '/include/CGAL/config/' + cgal_architecture, cgal_path + '/include']
cgal_libs = 			['CGAL', 'core++', 'mpfr', 'gmpxx', 'gmp', 'm']
cgal_link_flags =		['-Wl,-R' + cgal_path + '/lib/' + cgal_architecture]
cgal_lib_paths = 		[cgal_path + '/lib/' + cgal_architecture]

# Includes
cpppath =				['#/include'] + env['INCPATH'].split(':')
libpath = 				env['LIBPATH'].split(':')

# Append variables to the environment
env.Append				(CPPDEFINES = cgal_defines, 
						 CPPPATH = cpppath + cgal_cpppath,
						 LIBS = cgal_libs, 
						 LIBPATH = libpath + cgal_lib_paths, 
						 LINKFLAGS = cgal_link_flags)

# Build documentation (in docs/)
Default(env.Doxygen("Doxyfile"))

# Build stuff in src/, and export it
external_sources = 		SConscript(['src/SConscript'], exports = ['env'])

# Build examples/ and tools/
#SConscript				(['examples/SConscript',
#						  'tools/SConscript'],
#						 exports = ['env', 'external_sources'])
SConscript				(['examples/SConscript', 
						  'tests/SConscript'],
						 exports = ['env', 'external_sources'])

# vim: syntax=python
