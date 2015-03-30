require '/etc/cypress/server'

C = Cypress::ServerConfig

`cd #{C::DIRS[:www]}/Design`
#`puma -e production -b unix://#{C::DIRS[:runtime]}/design.sock`
`puma --bind tcp://127.0.0.1:7047 --daemon`

