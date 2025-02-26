#!/usr/bin/php -q
<?php
require('phpagi.php');

$agi = new AGI();
$call_id = $argv[1];  // ID da chamada passado como argumento

// Conexão ao banco de dados
$hostname = "localhost";
$username = "root";         
$password = "Mel@2307";     
$database = "asterisk";     

$conn = new mysqli($hostname, $username, $password, $database);

// Verificar conexão
if ($conn->connect_error) {
    $agi->verbose("Erro ao conectar ao banco de dados: " . $conn->connect_error);
    exit(1);
}

// Configura o charset para utf8mb4
if (!$conn->set_charset("utf8mb4")) {
    $agi->verbose("Erro ao configurar charset para utf8mb4: " . $conn->error);
    exit(1);
}

// Captura a duração da chamada
$billsec = $agi->get_variable('CDR(billsec)');  // Captura a duração da chamada

// Se a variável não estiver definida, use 0
$duracao = isset($billsec['data']) ? (int)$billsec['data'] : 0;  // Valor padrão

// Log para verificar valores
$agi->verbose("Duração (billsec): " . $duracao);

// Atualiza o status da chamada baseado na duração
if ($duracao > 1) {
    // Chamada considerada atendida
    $sql = "UPDATE chamadas SET status = 'Atendida', atendida = 1, duracao = '$duracao' WHERE id = $call_id";
    if ($conn->query($sql) === TRUE) {
        $agi->verbose("Status atualizado para Atendida. Duração: $duracao.");
    } else {
        $agi->verbose("Erro ao atualizar status para Atendida: " . $conn->error);
    }
} else {
    // Chamada não atendida
    $sql = "UPDATE chamadas SET status = 'Nao-Atendida', atendida = 0 WHERE id = $call_id";
    if ($conn->query($sql) === TRUE) {
        $agi->verbose("Status atualizado para Não Atendida.");
    } else {
        $agi->verbose("Erro ao atualizar status para Não Atendida: " . $conn->error);
    }
}

$conn->close();

// Retorna o controle para o Asterisk sem interromper a chamada
$agi->verbose("AGI script completado sem erros.");
exit(0);  // Certifique-se de sair com sucesso
?>
