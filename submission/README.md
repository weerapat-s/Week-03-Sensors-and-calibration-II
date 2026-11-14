# Week 03 - LDR and Calibration Submission

ชุดงานนี้เตรียมไว้สำหรับ KMITL FIGHT BOARD โดยใช้หัวสัญญาณ LDR ต่อเข้ากับ GPI34

## ไฟล์สำคัญ

- `arduino/ldr_calibration/ldr_calibration.ino` - ใช้กับ Arduino IDE
- `esp-idf/` - โปรเจกต์ ESP-IDF 5.x ตามข้อกำหนดในใบงาน
- `calibration.xlsx` - ตารางเก็บข้อมูล กราฟ และสมการ
- `data/ldr_measurements.csv` - แบบฟอร์มข้อมูลสำหรับสคริปต์
- `tools/calibrate.py` - คำนวณ Linear/Polynomial จาก CSV
- `report/REPORT.md` - รายงานและคำตอบท้ายใบงาน

## การต่อวงจร

```text
ช่อง LDR ของหัวต่อ PIR/LDR/IR Rx  --->  GPI34
```

ไม่ต้องต่อ 3.3V, GND หรือตัวต้านทานเพิ่มเมื่อใช้วงจร LDR บน KMITL FIGHT BOARD

## ขั้นตอนทำงาน

1. อัปโหลดโปรแกรม Arduino หรือ ESP-IDF
2. เก็บ ADC และ Lux จริงอย่างน้อย 5-7 จุด
3. กรอกค่าลง `calibration.xlsx`
4. เลือกโมเดลจาก R-squared และการทดสอบจุดใหม่
5. คัดลอกสัมประสิทธิ์ไปใส่ในโค้ดและตั้ง `CALIBRATION_READY` เป็น `1`
6. บันทึกผลจริง ภาพวงจร และภาพ Serial Monitor ลงรายงาน

## คำสั่ง ESP-IDF

```powershell
idf.py set-target esp32
idf.py build
idf.py -p COM8 flash monitor
```

## คำนวณจาก CSV

กรอก `data/ldr_measurements.csv` แล้วรันจากโฟลเดอร์ `submission`:

```powershell
python tools/calibrate.py data/ldr_measurements.csv
```

หมายเหตุ: ห้ามใช้ข้อมูลตัวอย่างแทนข้อมูลวัดจริงในการส่งผลการทดลอง
