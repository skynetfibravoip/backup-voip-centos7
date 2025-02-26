#!/usr/bin/php
<?php

// Importa a biblioteca phpagi.php
require('phpagi.php');

// Iniciar a interface AGI
$agi = new AGI();

// Configurações do banco de dados
$host = 'localhost';
$user = 'root';
$pass = 'Mel@2307';
$dbname = 'asterisk';

// Conectar ao banco de dados usando mysqli
$conn = new mysqli($host, $user, $pass, $dbname);

// Verificar se a conexão foi bem sucedida
if ($conn->connect_error) {
    $agi->verbose("Erro de conexão com o banco de dados: " . $conn->connect_error, 1);
    exit(1);
}

// Verificar se o ID foi passado como argumento
if ($argc < 2 || !is_numeric($argv[1])) {
    $agi->verbose("ID inválido ou não fornecido", 1);
    exit(1);
}

// Pegando o ID da chamada passado como argumento
$id = intval($argv[1]);
$agi->verbose("ID da chamada recebido: " . $id, 1);

// Consulta ao banco de dados para buscar informações na tabela 'chamadas' usando o ID
$sql = "SELECT status, duracao, atendida, arquivo_mp3, encaminha_chamada FROM chamadas WHERE id = ?";
$stmt = $conn->prepare($sql);
$stmt->bind_param("i", $id);
$stmt->execute();

// Variáveis para armazenar os resultados
$status = '';
$duracao = 0;
$atendida = 0;
$arquivo_mp3 = '';
$encaminha_chamada = 0;  // Valor padrão de encaminha_chamada

// Fazer o bind dos resultados às variáveis
$stmt->bind_result($status, $duracao, $atendida, $arquivo_mp3, $encaminha_chamada);

// Buscar os resultados
if ($stmt->fetch()) {
    $agi->verbose("Valor de status: " . $status, 1);
    $agi->verbose("Valor de duracao: " . $duracao, 1);
    $agi->verbose("Valor de atendida: " . $atendida, 1);
    $agi->verbose("Valor de arquivo_mp3: " . $arquivo_mp3, 1);
    $agi->verbose("Valor de encaminha_chamada: " . $encaminha_chamada, 1);

    // Retornar os dados para o Asterisk no formato desejado
    $agi->set_variable("status_chamada", $status);
    $agi->set_variable("duracao", $duracao);
    $agi->set_variable("atendida", $atendida);
    $agi->set_variable("arquivo_mp3", $arquivo_mp3);
    $agi->set_variable("encaminha_chamada", $encaminha_chamada);
} else {
    // Caso o ID não seja encontrado
    $agi->verbose("Chamada com ID " . $id . " não encontrada.", 1);
    $agi->set_variable("status_chamada", "Nao Encontrada");
    $agi->set_variable("duracao", "0");
    $agi->set_variable("atendida", "0");
    $agi->set_variable("arquivo_mp3", "");
    $agi->set_variable("encaminha_chamada", "0");
}

// Fechar a conexão com o banco de dados
$stmt->close();
$conn->close();

exit(0);
?>
