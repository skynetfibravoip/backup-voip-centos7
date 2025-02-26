<?php
// Configuração do banco de dados
$host = 'localhost';
$dbname = 'asterisk';
$user = 'root';
$pass = 'Mel@2307';

// Conectando ao banco de dados usando MySQLi
$conn = new mysqli($host, $user, $pass, $dbname);

// Verifica se a conexão foi bem-sucedida
if ($conn->connect_error) {
    die("Erro ao conectar ao banco de dados: " . $conn->connect_error);
}

// Função para encerrar a conexão (caso precise ser usada em outros pontos do script)
function fecharConexao($conn) {
    if ($conn) {
        $conn->close();
    }
}
?>
