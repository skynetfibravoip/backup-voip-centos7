#!/usr/bin/php -q
<?php
require 'phpagi.php';
$agi = new AGI();

$url = "http://45.70.210.10/ura/cliente.php?cpf_cnpj=$argv[1]";
$res = json_decode(file_get_contents($url));

if($res->id > ''){
    if($argv[3] > ''){
    // ================================= DESBLOQUEIO ==============================


    }
    // ================================= 2VIA de FATURA ==============================
    else if($argv[2] > ''){
        $url2via = "http://100.64.0.126/ura/boletos.php?cpf_cnpj=$argv[1]";
        $res2via = json_decode(file_get_contents($url2via));
        $whatsapp = $argv[2];
        $dia = substr($res2via[-0]->data_vencimento, -2);
        $mes = substr($res2via[-0]->data_vencimento, 5, 2);
        $ano = substr($res2via[-0]->data_vencimento, 0, 4);
        $data_venc = $dia.'/'.$mes.'/'.$ano;

        $msg = "*Cliente:* ".$res->nome.'%0A*Contrato:* '.$res->contrato.'%0A*Plano:* '.$res->plano.'%0A*Bloqueado(?):* '.$res->bloqueado.'%0A%0A📅 *Vencimento:* '.$data_venc.'%0A💰 *Valor:* R$ '.$res2via[-0]->valor.'%0A%0A_INFORMAÇÕES PARA PAGAMENTO_%0A%0A 💢 👇🏾 👇🏾 👇🏾 👇🏾 👇🏾 💢 %0A{split}📲 *Linha Digitavel:*{split}'.$res2via[-0]->linha_digitavel.'{split}💠 *Pix Qrcod:*{split}'.$res2via[-0]->copiacola.'{split}Para visualizar o boleto click no link abaixo:{split}https://skynetfibra.net.br/boleto/boleto.hhvm?titulo='.$res2via[-0]->titulo;
        $texto = str_replace( ' ','%20', $msg );

        $envia = "https://meujames.com/api/playsms/index.php?app=webservices&u=skynettelecomunicacao@gmail.com&p=7c390df17b262860f004cc2e98ac1aa41c0becf6&h=7c390df17b262860f004cc2e98ac1aa41c0becf6&ta=pv&op=pv&to='$argv[2]'&msg=$texto%26contrato=$res->login";
        $send = json_decode(file_get_contents($envia));
        $agi->exec('Set',"message", "ok");
        $agi->set_variable("message", "ok");
        }
    else{    
        $agi->exec('Set',"razaoSocial", $res->nome);
        $agi->set_variable("razaoSocial", $res->nome);
        $agi->exec('Set',"contratoId", $res->id);
        $agi->set_variable("contratoId", $res->id);
        $agi->exec('Set',"bloqueado", $res->bloqueado);
        $agi->set_variable("bloqueado", $res->bloqueado);
   
    }
}
else{

echo($retorno[0]);
    $agi->exec('Set',"contratoId", $res->id);
    $agi->set_variable("contratoId", $res->id);
}
?>
