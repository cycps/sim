# Cypress web-server /Design web-app

#require 'sinatra'
require 'fileutils'
#require './CyConfig.rb'
require 'io/console'
require 'securerandom'

#class Design
#  def new_exp(params)
#    "Creating #{params[:name]} ...\r\n"
#  end
#end

#class DesignApp < Sinatra::Base
#  @design = Design.new
#
#  post '/Design/New' do
#    @design.new_exp params
#  end
#
#end


def main
  dir = `pwd`.chop
  prog = "#{dir}/#{__FILE__}"
  me = `whoami`.chop
  puts "Hello, I am #{me}"
  if me != "cypress"
    puts "Switching to cypress account"
    puts `sudo -u cypress ruby #{prog}`
  else
    puts "Running as cypress"
  end

  #conf = CyConfig.new
  #conf.check

  #app = DesignApp.new
  #app.run!
end

main
