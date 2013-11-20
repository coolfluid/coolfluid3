import coolfluid as cf

printer = cf.root.create_component('PrintTimingTree', 'cf3.common.PrintTimingTree')
printer.root = cf.root
printer.execute()