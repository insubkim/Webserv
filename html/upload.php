<?php
// 업로드된 파일 정보 확인
if(isset($_FILES['file'])) {
    $file = $_FILES['file'];

    // 업로드된 파일의 정보
    $fileName = $file['name'];
    $fileTmpName = $file['tmp_name'];
    $fileSize = $file['size'];
    $fileError = $file['error'];

    // 파일 확장자 확인 (예: 이미지 파일만 허용하려면)
    $fileExt = pathinfo($fileName, PATHINFO_EXTENSION);
    $allowedExtensions = array("jpg", "jpeg", "png", "gif");

    if (in_array($fileExt, $allowedExtensions)) {
        if ($fileError === 0) {
            // 파일 크기 제한 설정 (예: 2MB로 설정)
            if ($fileSize <= 2 * 1024 * 1024) {
                // 파일 업로드 경로 설정
                $uploadPath = "uploads/" . $fileName;
                move_uploaded_file($fileTmpName, $uploadPath);
                echo "파일 업로드 성공!";
            } else {
                echo "파일이 너무 큽니다.";
            }
        } else {
            echo "파일 업로드 중 오류가 발생했습니다.";
        }
    } else {
        echo "올바른 파일 형식이 아닙니다. (jpg, jpeg, png, gif만 허용)";
    }
} else {
    echo "파일을 선택하지 않았습니다.";
}
?>
