# Cypress web-server /Design web-app

require 'trollop'
require 'sinatra'

opts = Trollop::options do
  opt :data_dir, 
      "Directory where experiment code repositories are placed",
      :type => :string,
      :default => "/var/lib/cypress/"

p opts

post '/Design/New' do
  "Creating #{params[:name]} ...\r\n"
end
