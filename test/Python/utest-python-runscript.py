import coolfluid as cf
import sys

# This test runs another python script using the ScriptEngine

root = cf.Core.root()

engine = root.create_component('PythonEngine', 'CF.Python.ScriptEngine')
script_file = open(sys.argv[1])
engine.execute_script(script_file.read())

print root.get_child('group').get_child('journal').uri()

