# ModuloPotenciaWFBL
Projeto Automasensor: ModuloPotenciaWFBL
# Escopo
Possui as mesmas funções dos demais módulo de potência, porém recebendo os comandos por WiFi e configurações por Bluetooth <br>
Aciona a PWM conforme o comando recebido pela wifi <br>
Conecta em uma rede WiFi com o nome e a senha recebidos por bluetooth <br>
O ip é fixo, e é recebido por bluetooth <br>
O equipamento cria um server para receber os comandos por telnet, na porta 23. <br> 
Após 62s sem receber uma atualização o módulo desliga a saída por segurança. <br
# Protocolo Telnet
Comando único: $,CARGA_PWM,\r\n <br>
CARGA_PWM: Valor da saída entre 0 e 4095 <br>
Resposta: $,RPM,\r\n <br>
Valor da RPM lida na entrada digital <br>
