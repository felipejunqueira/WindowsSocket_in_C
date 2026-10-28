#include <stdio.h>
#include <winsock2.h> // Padrao para sockets no windows

// Socket eh como um telefone. O primeiro liga, espera, pode receber a resposta, conversa
// se despedem e então desligam os telefones.

// Diretiva para compiladores como MSVC vincularem a biblioteca de rede (ws2_32).
// No GCC/MinGW, utilize a flag -lws2_32 no comando de compilacao.
#pragma comment(lib, "ws2_32.lib")

// Socket conecta IP, Porta e Protocolo(TCP)
// Para compilar: gcc servidor.c -o servidor.exe -lws2_32
// .\servidor.exe
// Em outro terminal: curl http://localhost:8080

int main(){
    // O código cria um servidor que escuta na porta 8080, aceita uma conexão, envia uma mensagem e fecha.
    // Inicialização do ambiente: exclusivo do windows
    WSADATA wsa; // Estrutura que guardará detalhes da implementação do Windows sockets
    // WSAStartup inicializa o uso da biblioteca. MAKEWORD(2,2) pede a versão 2.2
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Falha na inicializacao. Erro: %d\n", WSAGetLastError());
        return 1;
    }
    else printf("Ambiente Inicializado\n");

    // socket(Familia de endereco, tipo, protocolo)
    // AF_INET: IPv4;
    // SOCK_STREAM: TCP (conexao confiavel);
    // 0: Escolha automatica do protocolo (geralmente IP)
    SOCKET s;
    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == INVALID_SOCKET) {
        printf("Nao foi possivel criar o socket. Erro: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    else printf("Socket criado\n");

    struct sockaddr_in server; // Estrutura para configurar o IP e porta
    server.sin_family = AF_INET; // Define que usaremos IPv4
    server.sin_addr.s_addr = INADDR_ANY; // O servidor aceita conexoes de qualquer placa de rede do pc
    server.sin_port = htons(8080); // Porta 8080. htons converte o numero para "Network byte order", padroniza o formato

    // Ritual de conexao: Bind, Listen e Accept
    // Bind associa o socket 's' as configuracoes de ip/porta criadas
    if(bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Erro no bind: %d\n", WSAGetLastError()); // Casting do endereco de server
        closesocket(s);
        WSACleanup();
        return 1;
    }
    else printf("Bind feito!\n");

    // Listen coloca o socket em modo de espera. '3' eh o tamanho da fila de espera
    if(listen(s, 3) == SOCKET_ERROR) {
        printf("Erro no listen: %d\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return 1;
    }
    printf("Servidor iniciado. Aguardando conexao na porta 8080...\n");

    // Accept o programa "para" aqui e fica esperando alguem conectar
    // Quando alguem conecta, ele cria um NOVO socket (novo_socket) so para falar com esse cliente especifico
    int c = sizeof(struct sockaddr_in);
    struct sockaddr_in client;
    SOCKET novo_socket = accept(s, (struct sockaddr *)&client, &c);
    if (novo_socket == INVALID_SOCKET) {
        printf("Novo socket nao criado. Erro: %d\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return 1;
    }
    else printf("Novo socket criado!\n");

    char buffer[1024];
    // recv() limpa os dados que o curl enviou. se nao o windows reseta a conexao ao fechar
    int bytes_recebidos = recv(novo_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_recebidos == SOCKET_ERROR) {
        printf("Erro ao receber dados do cliente: %d\n", WSAGetLastError());
    } else {
        printf("Pedido recebido do cliente (%d bytes)!\n", bytes_recebidos);
    }

    char msg[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nOla! voce se conectou.\n";
    int size = sizeof(msg) - 1; // Tiramos o '\0';

    // Send envia os bytes da string atraves do socket do cliente
    int resultado_envio = send(novo_socket, msg, size, 0);
    if (resultado_envio == SOCKET_ERROR) {
        printf("Falha no envio da mensagem. Codigo de erro: %d\n", WSAGetLastError());
        closesocket(s);
        closesocket(novo_socket);
        WSACleanup();
        return 1;
    }
    else {
        printf("Mensagem enviada com sucesso. %d bytes transferidos :)\n", resultado_envio );
        // shutdown(socket, SD_SEND) avisa o termino ao cliente
        // Isso permite que o curl finalize a recepcao corretamente antes de fechar o socket
        shutdown(novo_socket, SD_SEND);
    }

    // Limpeza obrigatoria para nao deixar lixo na memoria/rede
    closesocket(novo_socket); // Fecha a conexao com o cliente
    closesocket(s); // Fecha o socket principal
    WSACleanup(); // Desliga a bib Winsock

    return 0;
}

// O http eh uma via de mao dupla. o curl envia o pedido (GET) do endereco e o servidor responde. o buffer recebe o pedido do curl. Esse eh o protcocolo TCP
// TCP eh a camada de transporte (socket, bind, listen, accept...)
// HTTP eh o conteudo, a camada de aplicacao (char msg[] = ...)
