from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
import sys
import setuptools
import os
from glob import glob

__version__ = '0.0.1'

# Note that this setup.py is somewhat nonstandard.  In the main BOOM repository
# stored on github, setup.py and the pybind11 bindings are kept in
# .../Interfaces/python/BayesBoom/... For setup.py to work the C++ code must be
# in a subdirectory below setup.py.
#
# A script in the top level of the BOOM project copies the BOOM source code
# into a build directory in a way that will make setup.py happy.  This file is
# intended to be run by that build script, and not directly from the
# repository.


class get_pybind_include(object):
    """Helper class to determine the pybind11 include path

    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. """

    def __init__(self, user=False):
        self.user = user

    def __str__(self):
        import pybind11
        return pybind11.get_include(self.user)


boom_headers = glob("*.hpp")

distributions_sources = glob("distributions/*.cpp")
distributions_headers = (
    ["distributions.hpp"]
    + glob("distributions/*.hpp")
    )

linalg_sources = glob("LinAlg/*.cpp")
linalg_headers = glob("LinAlg/*.hpp")

math_sources = glob("math/*.cpp") + glob("math/cephes/*.cpp")

numopt_sources = glob("numopt/*.cpp")
numopt_headers = ["{BOOM}/numopt.hpp"] + glob("numopt/*.hpp")

rmath_sources = glob("Bmath/*.cpp")
rmath_headers = glob("Bmath/*.hpp")

samplers_sources = glob("Samplers/*.cpp") + ["Samplers/Gilks/arms.cpp"]
samplers_headers = glob("Samplers/*.hpp")

stats_sources = glob("stats/*.cpp")
stats_headers = glob("stats/*.hpp")

targetfun_sources = glob("TargetFun/*.cpp")
targetfun_headers = glob("TargetFun/*.hpp")

utils_sources = glob("cpputil/*.cpp")
utils_headers = glob("cpputil/*.hpp")

models_sources = (
    glob("Models/*.cpp")
    + glob("Models/PosteriorSamplers/*.cpp")
    + glob("Models/Policies/*.cpp"))
models_headers = (
    glob("Models/*.hpp")
    + glob("Models/Policies/*.hpp")
    + glob("Models/PosteriorSamplers/*.hpp"))

# Specific model classes to be added later, glm's hmm's, etc.
bart_sources = (
    glob("Models/Bart/*.cpp")
    + glob("Models/Bart/PosteriorSamplers/*.cpp")
    )
bart_headers = (
    glob("Models/Bart/*.hpp")
    + glob("Models/Bart/PosteriorSamplers/*.hpp")
    )

glm_sources = (
    glob("Models/Glm/*.cpp")
    + glob("Models/Glm/PosteriorSamplers/*.cpp")
    )
glm_headers = (
    glob("Models/Glm/*.hpp")
    + glob("Models/Glm/PosteriorSamplers/*.hpp")
    )

hmm_sources = (
    glob("Models/HMM/*.cpp")
    + glob("Models/HMM/Clickstream/*.cpp")
    + glob("Models/HMM/Clickstream/PosteriorSamplers/*.cpp")
    + glob("Models/HMM/PosteriorSamplers/*.cpp")
    )
hmm_headers = (
    glob("Models/HMM/*.hpp")
    + glob("Models/HMM/Clickstream/*.hpp")
    + glob("Models/HMM/Clickstream/PosteriorSamplers/*.hpp")
    + glob("Models/HMM/PosteriorSamplers/*.hpp")
    )

hierarchical_sources = (
    glob("Models/Hierarchical/*.cpp")
    + glob("Models/Hierarchical/PosteriorSamplers/*.cpp")
    )
hierarchical_headers = (
    glob("Models/Hierarchical/*.hpp")
    + glob("Models/Hierarchical/PosteriorSamplers/*.hpp")
    )

impute_sources = (
    glob("Models/Impute/*.cpp")
    )
impute_headers = (
    glob("Models/Impute/*.hpp")
    )

irt_sources = (
    glob("Models/IRT/*.cpp")
    + glob("Models/IRT/PosteriorSamplers/*.cpp")
    )
irt_headers = (
    glob("Models/IRT/*.hpp")
    + glob("Models/IRT/PosteriorSamplers/*.hpp")
    )

mixture_sources = (
    glob("Models/Mixtures/*.cpp")
    + glob("Models/Mixtures/PosteriorSamplers/*.cpp")
    )
mixture_headers = (
    glob("Models/Mixtures/*.hpp")
    + glob("Models/Mixtures/PosteriorSamplers/*.hpp")
    )

nnet_sources = (
    glob("Models/Nnet/*.cpp")
    + glob("Models/Nnet/PosteriorSamplers/*.cpp")
    )
nnet_headers = (
    glob("Models/Nnet/*.hpp")
    + glob("Models/Nnet/PosteriorSamplers/*.hpp")
    )

point_process_sources = (
    glob("Models/PointProcess/*.cpp")
    + glob("Models/PointProcess/PosteriorSamplers/*.cpp")
    )
point_process_headers = (
    glob("Models/PointProcess/*.hpp")
    + glob("Models/PointProcess/PosteriorSamplers/*.hpp")
    )

state_space_sources = (
    glob("Models/StateSpace/*.cpp")
    + glob("Models/StateSpace/Filters/*.cpp")
    + glob("Models/StateSpace/PosteriorSamplers/*.cpp")
    + glob("Models/StateSpace/StateModels/*.cpp")
)
state_space_headers = (
    glob("Models/StateSpace/*.hpp")
    + glob("Models/StateSpace/Filters/*.hpp")
    + glob("Models/StateSpace/PosteriorSamplers/*.hpp")
    + glob("Models/StateSpace/StateModels/*.hpp")
)

time_series_sources = (
    glob("Models/TimeSeries/*.cpp")
    + glob("Models/TimeSeries/PosteriorSamplers/*.cpp")
    )
time_series_headers = (
    glob("Models/TimeSeries/*.hpp")
    + glob("Models/TimeSeries/PosteriorSamplers/*.hpp")
    )

boom_library_sources = (
    distributions_sources
    + linalg_sources
    + math_sources
    + numopt_sources
    + rmath_sources
    + samplers_sources
    + stats_sources
    + targetfun_sources
    + utils_sources
    + models_sources
    + bart_sources
    + glm_sources
    + hmm_sources
    + hierarchical_sources
    + impute_sources
    + irt_sources
    + mixture_sources
    + nnet_sources
    + point_process_sources
    + state_space_sources
    + time_series_sources
)

boom_extension_sources = (
    ["pybind11/module.cpp"]
    + glob("pybind11/Models/*.cpp")
    + glob("pybind11/Models/Glm/*.cpp")
    + glob("pybind11/Models/Impute/*.cpp")
    + glob("pybind11/Models/StateSpace/*.cpp")
    + glob("pybind11/Models/StateSpace/StateModels/*.cpp")
    + glob("pybind11/Models/TimeSeries/*.cpp")
    + glob("pybind11/LinAlg/*.cpp")
    + glob("pybind11/stats/*.cpp")
    + glob("pybind11/distributions/*.cpp")
)

boom_sources = boom_extension_sources + boom_library_sources

# ---------------------------------------------------------------------------
# From
# https://stackoverflow.com/questions/11013851/speeding-up-build-process-with-distutils
# monkey-patch for parallel compilation
def parallelCCompile(self, sources, output_dir=None, macros=None,
                     include_dirs=None, debug=0, extra_preargs=None,
                     extra_postargs=None, depends=None):

    # those lines are copied from distutils.ccompiler.CCompiler directly
    macros, objects, extra_postargs, pp_opts, build = self._setup_compile(
        output_dir, macros, include_dirs, sources, depends, extra_postargs)

    cc_args = self._get_cc_args(pp_opts, debug, extra_preargs)
    # parallel code
    N = 16  # number of parallel compilations
    import multiprocessing.pool

    def _single_compile(obj):
        try:
            src, ext = build[obj]
        except KeyError:
            return
        self._compile(obj, src, ext, cc_args, extra_postargs, pp_opts)

    # convert to list, imap is evaluated on-demand
    list(multiprocessing.pool.ThreadPool(N).imap(_single_compile, objects))
    return objects


import distutils.ccompiler
distutils.ccompiler.CCompiler.compile = parallelCCompile
# End of parallel compile "monkey patch"
# ---------------------------------------------------------------------------

ext_modules = [
    Extension(
        'cpp',
        sources=boom_sources,
        include_dirs=[
            os.getcwd(),
            # Path to pybind11 headers
            get_pybind_include(),
            get_pybind_include(user=True)
        ],
        language='c++'
    ),
]


# As of Python 3.6, CCompiler has a `has_flag` method.
# cf http://bugs.python.org/issue26689
def has_flag(compiler, flagname):
    """Return a boolean indicating whether a flag name is supported on
    the specified compiler.
    """
    import tempfile
    with tempfile.NamedTemporaryFile('w', suffix='.cpp') as f:
        f.write('int main (int argc, char **argv) { return 0; }')
        try:
            compiler.compile([f.name], extra_postargs=[flagname])
        except setuptools.distutils.errors.CompileError:
            return False
    return True


def cpp_flag(compiler):
    """Return the -std=c++[11/14/17] compiler flag.

    The newer version is prefered over c++11 (when it is available).
    """
    flags = ['-std=c++17', '-std=c++14', '-std=c++11']

    for flag in flags:
        if has_flag(compiler, flag):
            return flag

    raise RuntimeError('Unsupported compiler -- at least C++11 support '
                       'is needed!')


class BuildExt(build_ext):
    """A custom build extension for adding compiler-specific options."""
    c_opts = {
        'msvc': ['/EHsc'],
        'unix': [],
    }
    l_opts = {
        'msvc': [],
        'unix': [],
    }

    if sys.platform == 'darwin':
        darwin_opts = ['-stdlib=libc++',
                       '-mmacosx-version-min=10.14',
                       '-Wno-sign-compare']
        c_opts['unix'] += darwin_opts
        l_opts['unix'] += darwin_opts
    elif sys.platform == 'linux':
        c_opts['unix'] = ['-Wno-sign-compare']

    def build_extensions(self):
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])
        link_opts = self.l_opts.get(ct, [])
        if ct == 'unix':
            opts.append('-DVERSION_INFO="%s"' % self.distribution.get_version())  # noqa
            opts.append(cpp_flag(self.compiler))
            if has_flag(self.compiler, '-fvisibility=hidden'):
                opts.append('-fvisibility=hidden')

            # For deubgging purposes only.  Do not submit code with this option
            # present.
            # opts.append("-O0")
        elif ct == 'msvc':
            opts.append('/DVERSION_INFO=\\"%s\\"' % self.distribution.get_version())  # noqa
        for ext in self.extensions:
            ext.extra_compile_args = opts
            ext.extra_link_args = link_opts
        build_ext.build_extensions(self)


setup(
    name='BoomCpp',
    packages=find_packages(),
    version=__version__,
    author='Steven L. Scott',
    author_email='steve.the.bayesian@gmail.com',
    url='https://github.com/steve-the-bayesian/BOOM',
    description='Tools for Bayesian modeling.',
    long_description="""Boom stands for 'Bayesian object oriented modeling'.
    It is also the sound your computer makes when it crashes.

    The main part of the Boom library is formulated in terms of abstractions
    for Model, Data, Params, and PosteriorSampler.  A Model is primarily an
    environment where parameters can be learned from data.  The primary
    learning method is Markov chain Monte Carlo, with custom samplers defined
    for specific models.

    The archetypal Boom program looks something like this:

    import BoomBayes as Boom

    some_data = 3 * np.random.randn(100) + 7
    model = Boom.GaussianModel()
    model.set_data(some_data)
    precision_prior = Boom.GammaModel(0.5, 1.5)
    mean_prior = Boom.GaussianModel(0, 10**2)
    poseterior_sampler = Boom.GaussianSemiconjugateSampler(
        model, mean_prior, precision_prior)
    model.set_method(poseterior_sampler)
    niter = 100
    mean_draws = np.zeros(niter)
    sd_draws = np.zeros(niter)
    for i in range(100):
        model.sample_posterior()
        mean_draws[i] = model.mu()
        sd_draws[i] = model.sigma()

    """,
    ext_modules=ext_modules,
    install_requires=['pybind11>=2.3'],
    setup_requires=['pybind11>=2.3'],
    cmdclass={'build_ext': BuildExt},
    zip_safe=False,
)
