#Configuration file for Cypress App Server

require 'etc/cypress/server'

C = Cypress::ServerConfig

require "#{C::DIRS[:lib]}/Design.rb"
run DesignApp
