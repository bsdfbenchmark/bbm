# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'BBM: A BRDF Benchmark'
copyright = '2023, Pieter Peers'
author = 'Pieter Peers'
release = '0.5'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.intersphinx',
    'sphinx.ext.autosectionlabel',
    'sphinx.ext.todo',
    'sphinx.ext.coverage',
    'sphinx.ext.mathjax',
    'sphinx.ext.ifconfig',
    'sphinx.ext.viewcode',
    'breathe',
]

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']
highlight_language = 'c++'
html_css_files = [
    'css/custom.css',
]


# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_static_path = ['_static']
html_theme = 'sphinx_rtd_theme'
html_theme_options = {
  'display_version' : False,
  'prev_next_buttons_location' : 'bottom',
}
# html_logo = ''
# github_url = ''
# html_baseurl = ''

# -- Breathe--------------------------------------------------------------------------
breathe_default_project = "bbm"
breathe_show_define_initializer = False
