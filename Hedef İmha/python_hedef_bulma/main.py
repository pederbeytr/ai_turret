import cv2
import time
from serial import Serial

# Modeli yükle
face_cascade = cv2.CascadeClassifier('models/haarcascade_frontalface_default.xml')


def get_hit_point(rectangle):
    x, y, w, h = rectangle
    x2 = x + w
    y2 = y + h
    # Dikdörtgenin merkezini hesapla
    mid_point = [(x + x2) / 2, (y + y2) / 2]
    # Yüksekliği %20 artır ve geri döndür
    return int(mid_point[0]), int(mid_point[1] - 2 * h // 10)


def get_max_rect(faces):
    biggest_face = (0, 0, 0, 0)
    for (x, y, w, h) in faces:
        if w + h > biggest_face[2] + biggest_face[3]:
            biggest_face = (x, y, w, h)
    return biggest_face


def send_command(arduino, x, y):
    data = "X{0:d}Y{1:d}Z".format(x, y)
    arduino.write(str.encode(data))


def face_tracking(cap):
    arduino = Serial('COM3', 9600)
    time.sleep(2)
    print("Arduino'ya bağlantı sağlandı...")

    while True:
        _, img = cap.read()
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        # Yüzleri tespit et
        faces = face_cascade.detectMultiScale(gray, 1.1, 4)

        # En büyük yüzün etrafına dikdörtgen çiz
        (x, y, w, h) = get_max_rect(faces)
        cv2.rectangle(img, (x, y), (x + w, y + h), (255, 0, 0), 2)
        if x and y:
            hit_point = get_hit_point((x, y, w, h))
            cv2.circle(img, hit_point, 1, (0, 0, 255), 3)
            try:
                send_command(arduino, hit_point[0] + 50, hit_point[1])
            except:
                print("Hata")
                arduino.close()
                break
        else:
            try:
                send_command(arduino, -100, -100)
            except:
                print("Hata")
                arduino.close()
                break

        # Ekranda göster
        cv2.imshow('img', img)
        # Kaçış tuşuna basılırsa döngüyü sonlandır
        k = cv2.waitKey(30) & 0xff
        if k == 27:
            break

    # VideoCapture nesnesini serbest bırak
    cap.release()
    arduino.close()


if __name__ == '__main__':
    # Webcam'den video yakalamak için.
    camera_port = 0
    camera = cv2.VideoCapture(camera_port, cv2.CAP_DSHOW)
    # Test için videodan çeker
    # cap = cv2.VideoCapture('dosyaadi.mp4')

    face_tracking(camera)
    cv2.destroyAllWindows()
