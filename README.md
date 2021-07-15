Para a fase de testes serão considerados:
- Os servidores AS e FS
- Vários PDs (um por user, num dado instante) 
- Várias instâncias da aplicação User (uma por user, num dado instante) 
- Cada user completa uma operação de manipulação de ficheiros (list, retrieve, upload, delete ou remove) antes de iniciar a operação seguinte 

===================================================

Estão disponíveis servidores ***AS*** e ***FS*** a operar na máquina 'tejo.tecnico.ulisboa.pt' (***IP=193.136.138.142***):
- O ***AS*** espera comunicações (TCP/UDP) no porto ***58011***
- O ***FS*** espera ligações TCP no porto ***59000***

===================================================

Para teste dos servidores AS e FS desenvolvidos pelos alunos (a correr em máquinas na rede pública ou na rede local do laboratório LT5) podem ser invocadas remotamente aplicações PD e User, que serão executadas no “tejo”, utilizando scripts de comando aí existentes e que replicam os comandos introduzidos manualmente no teclado.
Para se executar os scripts disponíveis remotamente deve especificar os endereços IP e portos das máquinas onde estão os servidores AS e FS a testar, bem como o script a ser executado, dando o seguinte comando num browser:

http://tejo.tecnico.ulisboa.pt:58000/index.html?ASIP=AS_IP&ASPORT=AS_PORT&FSIP=FS_IP&FSPORT=FS_PORT&SCRIPT=script

Em que:
AS_IP – endereço IP da máquina onde está o servidor AS do grupo
AS_PORT – porto no qual o servidor AS espera por comunicações (UDP ou TCP) iniciadas pelo PD e User a correr no “tejo”
FS_IP – endereço IP da máquina onde está o servidor FS do grupoFS_PORT – porto no qual o servidor FS espera por ligações TCP iniciadas pelo User a correr no “tejo”
script – número, com um máximo de 2 dígitos, com o número do script de ensaio escolhido – os “scripts” estão descritos na secção “Scripts de ensaio” no Fénix
