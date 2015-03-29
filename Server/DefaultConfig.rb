#defualt Cypress App Server Config

module Cypress
  module ServerConfig
    DIRS = 
    { 
      :data => '/var/lib/cypress',
      :runtime => '/var/run/cypress',
      :log => '/var/log/cypress',
      :www => '/var/www/cypress',
      :etc => '/etc/cypress',
      :lib => '/usr/lib/cypress',
    } 
  end
end
