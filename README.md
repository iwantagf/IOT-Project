# PROJECT IOT: BÃI ĐỖ XE THÔNG MINH

## 1. Danh sách thiết bị cần chuẩn bị thêm

> **Phần cứng hiện có:** Bộ kit cơ bản, vi điều khiển ESP32 và module đầu đọc RFID RC522.

### 1.1. Nhóm thiết bị cho Thanh chắn (Barie)
* **Động cơ Servo SG90 hoặc MG996R:** Dùng làm cơ cấu quay nâng/hạ thanh chắn mô hình *(SG90 nhẹ, rẻ, dễ test trên breadboard; MG996R bánh răng kim loại, khỏe hơn nếu gắn thanh chắn dài hoặc nặng)*.
* **Thanh chắn mô hình & trụ đỡ:** Cắt từ mica, thanh gỗ nhẹ, ống nhựa nhỏ hoặc in 3D để gắn trực tiếp lên cánh tay đòn của servo.
* **Nguồn cấp riêng 5V cho Servo (Khuyên dùng):** Tránh lấy dòng lớn trực tiếp từ chân 5V/VIN của ESP32 khi servo tải nặng gây sụt áp làm reset mạch. Nên dùng module nguồn breadboard 3.3V/5V hoặc adapter 5V riêng *(lưu ý nối chung **GND** với ESP32)*.

### 1.2. Nhóm cảm biến & hiển thị *(Tùy chọn mở rộng)*
* ~~**Cảm biến hồng ngoại vật cản (IR Obstacle Sensor):** Đặt trước/sau barie để phát hiện xe dừng chờ quẹt thẻ hoặc kiểm tra xe đã đi qua hẳn chưa trước khi hạ barie.~~
* ~~**Cảm biến siêu âm HC-SR04:** Đo khoảng cách và phát hiện có xe tại từng ô đỗ cụ thể hoặc đo khoảng cách xe trước cổng vào.~~
* ~~**Màn hình OLED 0.96 inch (I2C - SSD1306) hoặc LCD 16x2 I2C:** Hiển thị trạng thái số chỗ trống, thông báo *"Mời vào"*, *"Thẻ hợp lệ"* hoặc *"Bãi đầy"* trực tiếp tại cổng.~~
* ~~**LED xanh / đỏ (hoặc module LED/Relay):** Báo hiệu làn mở/đóng hoặc bật đèn chiếu sáng khu vực đỗ xe.~~

---

## 2. TODO List chi tiết theo từng giai đoạn (Phục vụ báo cáo)

### Giai đoạn 1: Khởi động & Kiểm tra phần cứng nền tảng
- [x] **Hoàn tất kiểm tra ESP32 & LED:** Xác nhận build/flash thành công trên môi trường phát triển và điều khiển được các bóng LED tín hiệu.
- [ ] **Chốt sơ đồ chân (Pinout) toàn hệ thống:** Lập bảng phân công chân GPIO cho RFID RC522 (SPI), Servo, cảm biến IR/siêu âm, màn hình I2C; tránh các chân strapping nhạy cảm và nhóm chân nối Flash nội (`D6–D11`).
- [x] **Kiểm tra & đọc UID thẻ RFID RC522:** Đấu nối RC522 với ESP32 qua chuẩn SPI, test đọc mã UID của từng thẻ/móc khóa RFID và in ra log kiểm tra.

### Giai đoạn 2: Điều khiển thanh chắn & Cảm biến làn xe
- [ ] **Lắp ráp cơ cấu thanh chắn Servo:** Gắn thanh chắn vào trục servo, căn chỉnh góc quay mở (ví dụ `90°`) và góc đóng (`0°`) đảm bảo không bị kẹt cơ khí.
- [ ] **Tích hợp cảm biến phát hiện xe (IR / HC-SR04):** Đọc tín hiệu xe vào làn, xử lý chống nhiễu/lọc tín hiệu giả để tránh barie đóng/mở liên tục.
- [ ] **Xây dựng logic đóng/mở barie an toàn:** Chỉ mở barie khi quẹt đúng thẻ hợp lệ (và bãi còn chỗ trống), giữ mở trong lúc xe đang đi qua, chỉ hạ barie khi cảm biến xác nhận xe đã rời khỏi vùng nguy hiểm.

### Giai đoạn 3: Quản lý chỗ đỗ & Hiển thị tại chỗ
- [ ] **Quản lý trạng thái từng ô đỗ:** Đếm số lượng xe hiện tại, cập nhật trạng thái còn trống/hết chỗ theo thời gian thực khi xe vào hoặc rời bãi.
- [ ] **Hiển thị thông tin lên OLED/LCD:** Hiển thị số slot trống, trạng thái quẹt thẻ và cảnh báo khi bãi hết chỗ.
- [ ] **Bổ sung âm thanh/đèn báo hiệu:** Còi bíp xác nhận thẻ hợp lệ, cảnh báo thẻ sai hoặc hết chỗ; tùy chọn tự bật đèn chiếu sáng khi trời tối (nếu dùng quang trở).

### Giai đoạn 4: Kết nối IoT, Dữ liệu & Giao diện giám sát
- [ ] **Kết nối Wi-Fi cho ESP32:** Cấu hình kết nối mạng ổn định và cơ chế tự động kết nối lại khi rớt mạng.
- [ ] **Gửi/nhận dữ liệu lên nền tảng IoT/Server:** Đẩy sự kiện xe vào/ra, UID thẻ, mốc thời gian và số chỗ trống qua giao thức MQTT hoặc HTTP/REST API.
- [ ] **Xây dựng Dashboard / Giao diện theo dõi:** Hiển thị trạng thái bãi đỗ trực quan, lịch sử ra/vào và hỗ trợ thao tác từ xa (mở barie thủ công, quản lý danh sách thẻ) nếu cần.

### Giai đoạn 5: Kiểm thử, Tối ưu & Hoàn thiện báo cáo
- [ ] **Kiểm thử kịch bản thực tế & ngoại lệ:** Test các tình huống quẹt thẻ lạ, quẹt thẻ liên tục, bãi đầy xe, mất kết nối Wi-Fi, sụt áp nguồn khi servo quay.
- [ ] **Tối ưu mã nguồn & chia module:** Tách riêng các class/module điều khiển RFID, Servo, cảm biến, màn hình và kết nối mạng để dễ bảo trì và trình bày.
- [ ] **Hoàn thiện tài liệu báo cáo:** Tổng hợp sơ đồ khối hệ thống, sơ đồ đấu nối chân, lưu đồ thuật toán, bảng linh kiện/chi phí, ảnh/video demo thực tế và đánh giá hạn chế/hướng phát triển.