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

proc ssh1 {gateway_user gateway_host} {
  expect {
    -re "^.*($gateway_user).*\\$" {
      return 1
    }
    "assword: " {
      send_user "#$gateway_host#$gateway_user#\n"
      exit
    }
    "(yes/no)? " {
      send -- "yes\r"
      -re "^.*($gateway_user).*\\$" {
        return 1
      }
      "assword: " {
        send_user "#$gateway_host#$gateway_user#\n"
        exit
      }
      default {
        exit
      }
    }
    default {
      exit
    }
  }
}

proc ssh2 {distant_user distant_host} {
  send -- "/usr/bin/ssh -t -t $distant_user@$distant_host\r"
  expect {
    -re "^.*($distant_user).*\\$" {
      return 1
    }
    "assword: " {
      send_user "#$distant_host#$distant_user#\n"
      exit
    }
    "(yes/no)? " {
      send -- "yes\r"
      -re "^.*($distant_user).*\\$" {
        return 1
      }
      "assword: " {
        send_user "#$distant_host#$distant_user#\n"
        exit
      }
      default {
        exit
      }
    }
    default {
      exit
    }
  }
}

proc ssh3 {local_user local_host local_port distant_port} {
  send -- "/usr/bin/ssh -t -t -R $local_port:localhost:$distant_port $local_user@$local_host\r"
  expect {
    -re "^.*($local_user).*\\$" {
      return 1
    }
    "assword: " {
      send_user "\n#$local_host#$local_user#\n"
      exit
    }
    "(yes/no)? " {
      send -- "yes\r"
      -re "^.*($local_user).*\\$" {
        return 1
      }
      "assword: " {
        send_user "\n#$local_host#$local_user#\n"
        exit
      }
      default {
        exit
      }
    }
    default {
      exit
    }
  }
}

spawn /usr/bin/ssh -t -t $gateway_user@$gateway_host

set ret [ssh1 $gateway_user $gateway_host]
set ret [ssh2 $distant_user $distant_host]
set ret [ssh3 $local_user $local_host $local_port $distant_port]
interact
