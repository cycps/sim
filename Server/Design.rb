require 'sinatra/base'

module Design
  def new_exp(params)
    "Creating #{params[:name]} ...\r\n"
  end
end

class DesignApp < Sinatra::Base
  include Design

  get '/Design/Status' do
    "Running & Gunning"
  end

  post '/Design/New' do
    new_exp params
  end

  set :port, 7047
  run!
end
