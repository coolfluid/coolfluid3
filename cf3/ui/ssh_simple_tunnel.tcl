#!/usr/bin/expect -f
set timeout -1
set local_port [lindex $argv 0]
set distant_port [lindex $argv 1]
set gateway_host [lindex $argv 2]
set gateway_user [lindex $argv 3]
set gateway_pass [lindex $argv 4]
set distant_host [lindex $argv 5]
spawn /usr/bin/ssh -t -t -L $local_port:$distant_host:$distant_port $gateway_user@$gateway_host
expect {
  "> " { }
  "$ " { }
  "assword: " {
    send "$gateway_pass\n"
    expect {
      "> " { }
      "$ " { }
      default {
        exit
      }
    }
  }
  "(yes/no)? " {
    send "yes\n"
    expect {
      "> " { }
      "$ " { }
      "assword: " {
        send "$gateway_pass\n"
        expect {
          "> " { }
          "$ " { }
          default {
            exit
          }
        }
      }
      default {
        exit
      }
    }
  }
  default {
    exit
  }
}
interact
