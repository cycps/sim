# Cypress web-server /Design web-app

require 'sinatra'
require 'fileutils'
require './DefaultConfig.rb'

class CyConfig
  def check
    check_acct
    check_data_dir
  end

  def check_acct
    begin
      cyp_uid = Process::UID.from_name("cypress")
    rescue
      puts "cypress user account does not exist, creating it"
    end
  end

  def check_data_dir
    ddir = Cypress::ServerConfig::DATA[:base_dir]
    if not Dir.exists? ddir 
      puts "Data directory #{ddir} does not exist, creating it"
      #FileUtils.mkdir_p ddir
      #TODO need to create a cypress user + permissions
      #
      # 1: detect user, if not cyp switch to cyp if no cyp create and switch
      # 2: no make dirs and mod them for use with cyp
      `sudo mkdir -p #{ddir}`
    end
  end
end

class Design
  def new_exp(params)
    "Creating #{params[:name]} ...\r\n"
  end
end

class DesignApp < Sinatra::Base
  @design = Design.new

  post '/Design/New' do
    @design.new_exp params
  end

end


def main
  conf = CyConfig.new
  conf.check

  #app = DesignApp.new
  #app.run!
end

main
