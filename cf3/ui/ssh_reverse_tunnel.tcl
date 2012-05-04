#!/usr/bin/expect -f
set timeout -1
set local_host [lindex $argv 0]
set gateway_host [lindex $argv 1]
set distant_host [lindex $argv 2]
set local_port [lindex $argv 3]
set distant_port [lindex $argv 4]
set local_user [lindex $argv 5]
set gateway_user [lindex $argv 6]
set distant_user [lindex $argv 7]
set local_pass [lindex $argv 8]
set gateway_pass [lindex $argv 9]
set distant_pass [lindex $argv 10]

proc ssh1 {gateway_user gateway_pass} {
  expect {
    -re "^.*($gateway_user).*\\$" {
      return 1
    }
    "assword: " {
      send -- "$gateway_pass\r"
      expect {
      	-re "^.*($gateway_user).*\\$" {
      	  return 1
      	}
      	default {
      	  return 0
      	}
      }
    }
    "(yes/no)? " {
      send -- "yes\r"
      -re "^.*($gateway_user).*\\$" {
        return 1
      }
      "assword: " {
        send -- "$gateway_pass\r"
        expect {
      	  -re "^.*($gateway_user).*\\$" {
      	    return 1
      	  }
      	  default {
      	    return 0
      	  }
        }
      }
    }
  }
}

proc ssh2 {distant_user distant_host distant_pass} {
  send -- "/usr/bin/ssh -t -t $distant_user@$distant_host\r"
  expect {
    -re "^.*($distant_user).*\\$" {
      return 1
    }
    "assword: " {
      send -- "$distant_pass\r"
      expect {
      	-re "^.*($distant_user).*\\$" {
      	  return 1
      	}
      	default {
      	  return 0
      	}
      }
    }
    "(yes/no)? " {
      send -- "yes\r"
      -re "^.*($distant_user).*\\$" {
        return 1
      }
      "assword: " {
        send -- "$distant_pass\r"
        expect {
      	  -re "^.*($distant_user).*\\$" {
      	    return 1
      	  }
      	  default {
      	    return 0
      	  }
        }
      }
    }
  }
}

proc ssh3 {local_user local_host local_pass local_port distant_port} {
  send -- "/usr/bin/ssh -t -t -R $local_port:localhost:$distant_port $local_user@$local_host\r"
  expect {
    -re "^.*($local_user).*\\$" {
      return 1
    }
    "assword: " {
      send -- "$local_pass\r"
      expect {
      	-re "^.*($local_user).*\\$" {
      	  return 1
      	}
      	default {
      	  return 0
      	}
      }
    }
    "(yes/no)? " {
      send -- "yes\r"
      -re "^.*($local_user).*\\$" {
        return 1
      }
      "assword: " {
        send -- "$local_pass\r"
        expect {
      	  -re "^.*($local_user).*\\$" {
      	    return 1
      	  }
      	  default {
      	    return 0
      	  }
        }
      }
    }
  }
}

spawn /usr/bin/ssh -t -t $gateway_user@$gateway_host

set ret [ssh1 $gateway_user $gateway_pass]
if {$ret == 0} {
  exit
}
set ret [ssh2 $distant_user $distant_host $distant_pass]
if {$ret == 0} {
  exit
}
set ret [ssh3 $local_user $local_host $local_pass $local_port $distant_port]
if {$ret == 0} {
  exit
}
interact
