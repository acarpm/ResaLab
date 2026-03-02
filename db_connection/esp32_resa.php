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
        WHERE end_time >= NOW()
            AND device_id = ?
            AND status IN ('à venir', 'en cours')
        ORDER BY end_time ASC
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

function changeReservationState($reservationID, $newStateCode, $conn) {
    $newState = "";
    switch ($newStateCode) {
        case "1": // Get next reservation
            $newState = "en cours";
            break;
        case "2": // Change reservation state
            $newState = "annulé";
            break;
        default:
            $response = "400\n";
    }

    $stmt = $conn->prepare("UPDATE booking SET status = ? WHERE booking_id = ?");
    if (!$stmt) {
        return false;
    }
    $stmt->bind_param("si", $newState, $reservationID);
    $result = $stmt->execute();


    $stmt = $conn->prepare("SELECT status FROM booking WHERE booking_id = ?");
    if (!$stmt) {
        return false;
    }
    $stmt->bind_param("i", $reservationID);
    $stmt->execute();
    $stmt->bind_result($currentStatus);
    if($stmt->fetch()) {
        if ($currentStatus === $newState) {
            $response = "006\n"; // State changed successfully
        } else {
            $response = "007\n"; // State not changed
        }
    } else {
        $response = "007\n"; // State not changed (reservation not found)
    }
    $stmt->close();
    $conn->close();

    return $response;
}

function getReservationName($reservationID, $conn) {
    $stmt = $conn->prepare("SELECT user_id FROM booking WHERE booking_id = ?");
    if (!$stmt) {
        return false;
    }

    $stmt->bind_param("i", $reservationID);
    $stmt->execute();
    $stmt->bind_result($user_id);
    if ($stmt->fetch()) {
        $stmt->close();

        $stmt = $conn->prepare("SELECT full_name, name FROM user WHERE user_id = ?");
        if (!$stmt) {
            return false;
        }

        $stmt->bind_param("i", $user_id);
        $stmt->execute();
        $stmt->bind_result($user_full_name, $user_name);
        if ($stmt->fetch()) {
            $stmt->close();
            $conn->close();
            return "008" . $user_full_name . "&" . $user_name . "\n";
        } else {
            $stmt->close();
            $conn->close();
            return "009\n"; // User not found
        }
    } else {
        $stmt->close();
        $conn->close();
        return "005\n"; // Reservation not found
    }
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
        case "100": // Test connection
            $response = "200\n";
            break;
        case "102": // Get next reservation
            $response = handleReservationRequest($payload, $conn);
            break;
        case "103": // Change reservation state
            if (strlen($payload) < 7) {
                http_response_code(400);
                $response = "Format invalide\n";
                break;
            }
            $reservationID_str = intval(substr($payload, 0, 5));
            $newState_code = intval(substr($payload, 5));
            $response = changeReservationState($reservationID_str, $newState_code, $conn);
            break;
        case "104": // Get reservation name
            $reservationID_str = intval($payload);
            $response = getReservationName($reservationID_str, $conn);
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
