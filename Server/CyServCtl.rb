# Cypress web-server /Design web-app

require 'trollop'
require 'fileutils'
#require './CyConfig.rb'
require '/etc/cypress/server'
require 'net/http'

$C = Cypress::ServerConfig

SUB_COMMANDS = %w(status start stop)
$global_opts = Trollop::options do
  banner "cyserv [status start stop]"
  stop_on SUB_COMMANDS
end

def status
    begin
      design_status = Net::HTTP.get('localhost', '/Design/Status', 7047)
      puts "Design: \t\t" + design_status
    rescue
      puts "Design: \t\tDown"
    end
end

def start
    puts "starting server"
    `sudo -E -u cypress ruby #{$C::DIRS[:lib]}/Design/launch.rb`
end

def stop
    puts "stopping server"
    response = Net::HTTP.get('localhost', '/Design/Status', 7047).split
    if(response[0] == "Running")
      puts "Shutting down /Design running @pid #{response[1]}"
      `sudo kill #{response[1]}`
    end
end

cmd = ARGV.shift
case cmd
  when "status"; status
  when "start"; start
  when "stop"; stop
  else; Trollop::die "unknown subcommand #{cmd}"
end


#conf = CyConfig.new
#conf.check

#puts `sudo -E -u cypress ruby Design.rb`

