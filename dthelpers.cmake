# dthelpers.cmake
#
# --------------------------------------------------------------------------
# Resolves VAR_NAME to the dtcore repo directory (the directory containing this file).
# In more complex projects dthelpers_resolve reads an environment variable of the same
# name; here we can simply derive the path from CMAKE_CURRENT_LIST_DIR.

function(dthelpers_resolve var_name help_text)
    set(${var_name} "${CMAKE_CURRENT_FUNCTION_LIST_DIR}" PARENT_SCOPE)
endfunction()
