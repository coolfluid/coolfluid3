#!/usr/bin/expect -f
set timeout -1
set local_port [lindex $argv 0]
set distant_port [lindex $argv 1]
set gateway_host [lindex $argv 2]
set gateway_user [lindex $argv 3]
set distant_host [lindex $argv 4]
spawn /usr/bin/ssh -t -t -L $local_port:$distant_host:$distant_port $gateway_user@$gateway_host
expect {
  "> " { }
  "$ " { }
  "assword: " {
    send_user "\n#$gateway_host#$gateway_user#\n"
    exit
  }
  "(yes/no)? " {
    send "yes\n"
    expect {
      "> " { }
      "$ " { }
      "assword: " {
        send_user "\n#$gateway_host#$gateway_user#\n"
        exit
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
