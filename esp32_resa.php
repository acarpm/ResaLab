<?php
session_start();
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

require_once '../db_config.php'; // Doit définir $conn (MySQLi)

// 1. Gestion GET : juste répondre 200 OK
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    echo "\n";
    exit; // rien d'autre à faire
}

function handleReservationRequest($device_id_str, $conn) {

    if (!ctype_digit($device_id_str)) {
        http_response_code(400);
        return "Format invalide";
    }

    $device_id = intval($device_id_str);
    $stmt = $conn->prepare("
        SELECT booking_id, start_time, status, end_time
        FROM booking
        WHERE start_time >= NOW()
            AND device_id = ?
        ORDER BY start_time ASC
        LIMIT 1
    ");

    if (!$stmt) {
        http_response_code(500);
        return "Erreur SQL";
    }

    $stmt->bind_param("i", $device_id);
    $stmt->execute();
    $stmt->bind_result($booking_id, $start_time, $status, $finish_time);

    if ($stmt->fetch()) {
        $start_timestamp = strtotime($start_time);
        $finish_timestamp = strtotime($finish_time);

        $formatted_booking_id = str_pad(strval($booking_id), 5, "0", STR_PAD_LEFT);
        if ($status === "à venir") {
            $state = "1";
        } else {
            $state = "0";
        }
        $response = "004" . $formatted_booking_id . $state . $start_timestamp . $finish_timestamp . "\n";
    } else {
        $response = "005\n";
    }

    $stmt->close();
    $conn->close();
    return $response;
}

// 2. Gestion POST : récupérer la donnée brute
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Récupérer le corps brut POST
    $input = file_get_contents('php://input');
    $input = trim($input);

    $response = "";

    $commande = substr($input, 0, 3);
    $payload = substr($input, 3);

    switch ($commande) {
        case "102": // Get next reservation
            $response = handleReservationRequest($payload, $conn);
            break;
        case "100": // Test connection
            $response = "200\n";
            break;
        default:
            $response = "400\n";
    }

    echo $response;

    exit;
}

// Méthode HTTP non supportée
http_response_code(405);
die("Méthode HTTP non autorisée");
