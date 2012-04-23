def cf_check(test):
  """if the first argument is False it raise an exception with a default message, you can specify the message in the second argument (optional)"""
  if not test:
    raise Exception('check failed')

def cf_check(test,message):
  if not test:
    raise Exception(message)

def cf_check_close(a,b,close):
  """if the absolute difference between the two firsts argument is smaller then the third argument it raise an exception with a default message, you can specify the message in the fourth argument (optional)"""
  if abs(a-b)>close:
    raise Exception('check close failed (first value:'+str(a)+', second value:'+str(b)+')')

def cf_check_close(a,b,close,message):
  if abs(a-b)>close:
    raise Exception(message+' (first value:'+str(a)+', second value:'+str(b)+')')

def cf_check_equal(a,b):
  """if the two firsts arguments are not equal it raise an exception with a default message, you can specify the message in the third argumnt (optional)"""
  if not a==b:
    raise Exception('check equal failed (first value:'+str(a)+', second value:'+str(b)+')')

def cf_check_equal(a,b,message):
  if not a==b:
    raise Exception(message+' (first value:'+str(a)+', second value:'+str(b)+')')

def cf_error():
  """raise an exception with a default message, tou can specify the message in the first argument (optional)"""
  raise Exception('error occured')

def cf_error(message):
  raise Exception(message)
