# Cypress web-server /Design web-app

require 'trollop'
require 'sinatra'
require 'fileutils'

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

def init(opts)
  #TODO need to ask permission to eleveate to make this dir if necessary
  ddir = opts[:data_dir]
  if not Dir.exists? ddir
    puts "Data directory #{ddir} does not exist, creating it"
    FileUtils.mkdir_p ddir
  end
end


def main
  opts = Trollop::options do
    opt :data_dir, 
        "Directory where experiment code repositories are placed",
        :type => :string,
        :default => "/var/lib/cypress/"
  end
  p opts
  init opts
  app = DesignApp.new
  app.run!
end

main
