#!/usr/bin/env python3
"""คำนวณสมการ Linear และ Polynomial degree 2 จากไฟล์ CSV จริง."""

import csv
import math
import sys
from pathlib import Path


def solve_3x3(matrix, vector):
    a = [row[:] + [value] for row, value in zip(matrix, vector)]
    for pivot in range(3):
        best = max(range(pivot, 3), key=lambda row: abs(a[row][pivot]))
        a[pivot], a[best] = a[best], a[pivot]
        if abs(a[pivot][pivot]) < 1e-12:
            raise ValueError("ข้อมูลไม่เพียงพอหรือค่า ADC ซ้ำกันมากเกินไป")
        divisor = a[pivot][pivot]
        a[pivot] = [value / divisor for value in a[pivot]]
        for row in range(3):
            if row == pivot:
                continue
            factor = a[row][pivot]
            a[row] = [a[row][col] - factor * a[pivot][col] for col in range(4)]
    return [a[row][3] for row in range(3)]


def r_squared(actual, predicted):
    mean = sum(actual) / len(actual)
    total = sum((value - mean) ** 2 for value in actual)
    residual = sum((a - p) ** 2 for a, p in zip(actual, predicted))
    return 1.0 - residual / total if total else 1.0


def load_points(path):
    points = []
    with path.open(encoding="utf-8-sig", newline="") as stream:
        for row in csv.DictReader(stream):
            adc = (row.get("adc") or "").strip()
            lux = (row.get("lux_mobile") or row.get("lux") or "").strip()
            if adc and lux:
                points.append((float(adc), float(lux)))
    if len(points) < 3:
        raise ValueError("ต้องมีข้อมูล ADC และ Lux จริงอย่างน้อย 3 จุด (แนะนำ 5-7 จุด)")
    return points


def main():
    csv_path = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("ldr_measurements.csv")
    points = load_points(csv_path)
    x = [point[0] for point in points]
    y = [point[1] for point in points]
    n = len(points)

    sx, sy = sum(x), sum(y)
    sxx = sum(value * value for value in x)
    sxy = sum(a * b for a, b in points)
    denominator = n * sxx - sx * sx
    if abs(denominator) < 1e-12:
        raise ValueError("ค่า ADC ต้องแตกต่างกัน")

    m = (n * sxy - sx * sy) / denominator
    b = (sy - m * sx) / n
    linear_pred = [m * value + b for value in x]
    linear_r2 = r_squared(y, linear_pred)

    sx2 = sum(value ** 2 for value in x)
    sx3 = sum(value ** 3 for value in x)
    sx4 = sum(value ** 4 for value in x)
    sx2y = sum((a ** 2) * b_value for a, b_value in points)
    poly_a, poly_b, poly_c = solve_3x3(
        [[sx4, sx3, sx2], [sx3, sx2, sx], [sx2, sx, n]],
        [sx2y, sxy, sy],
    )
    poly_pred = [poly_a * value * value + poly_b * value + poly_c for value in x]
    poly_r2 = r_squared(y, poly_pred)

    print(f"จำนวนจุดวัด: {n}")
    print(f"Linear: lux = {m:.10g} * adc + {b:.10g}   R^2={linear_r2:.6f}")
    print(
        "Polynomial: lux = "
        f"{poly_a:.10g} * adc^2 + {poly_b:.10g} * adc + {poly_c:.10g}"
        f"   R^2={poly_r2:.6f}"
    )
    print("\nเลือกโมเดลที่ R^2 สูงและผลทดสอบจุดใหม่คลาดเคลื่อนต่ำกว่า")
    print("\nค่าที่นำไปใส่ในโค้ด:")
    print(f"LINEAR_M = {m:.10g}f;")
    print(f"LINEAR_B = {b:.10g}f;")
    print(f"POLY_A = {poly_a:.10g}f;")
    print(f"POLY_B = {poly_b:.10g}f;")
    print(f"POLY_C = {poly_c:.10g}f;")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, KeyError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        sys.exit(1)
