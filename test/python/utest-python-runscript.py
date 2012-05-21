from coolfluid import *
# This test runs another python script using the ScriptEngine

root = Core.root()
engine = root.get_child('Tools').get_child('Python').get_child('ScriptEngine')

script_file = open(sys.argv[1])
engine.execute_script(script_file.read())

print root.get_child('group').get_child('journal').uri()

