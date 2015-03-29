require 'etc/cypress/server'

C = Cypress::ServerConfig

`cd #{C::DIRS[:www]}/design`
`puma -e production -b unix://#{C::DIRS[:runtime]}/design.sock`
