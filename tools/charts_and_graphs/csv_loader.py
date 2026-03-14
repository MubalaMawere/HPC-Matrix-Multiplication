from __future__ import annotations

import csv
from collections import defaultdict
from pathlib import Path

from charts_and_graphs.models import ResultRow


class ResultDataLoader:
    def __init__(self, csv_path: Path) -> None:
        self.csv_path = csv_path

    def load_rows(self) -> list[ResultRow]:
        if not self.csv_path.exists():
            raise SystemExit(f"CSV file not found: {self.csv_path}")

        rows: list[ResultRow] = []

        with self.csv_path.open(newline="", encoding="utf-8") as csv_file:
            reader = csv.DictReader(csv_file)

            for index, row in enumerate(reader):
                rows.append(
                    ResultRow(
                        index=index,
                        mode=row["mode"].strip(),
                        matrix_size=int(row["matrix_size"]),
                        threads=int(row["threads"]),
                        sample_count=self._parse_optional_int(row.get("sample_count", "")),
                        sample_times=self._parse_sample_times(row.get("sample_times", "")),
                        execution_seconds=float(row["execution_seconds"]),
                        speedup=self._parse_optional_float(row["speedup"]),
                        efficiency=self._parse_optional_float(row["efficiency"]),
                        verified=row["verified"].strip(),
                    )
                )

        return rows

    def load_latest_rows_by_size(self) -> dict[int, list[ResultRow]]:
        rows_by_size: dict[int, list[ResultRow]] = defaultdict(list)

        for row in self.load_rows():
            rows_by_size[row.matrix_size].append(row)

        latest: dict[int, list[ResultRow]] = {}

        for matrix_size, rows in rows_by_size.items():
            latest[matrix_size] = self._select_rows_for_report(rows)

        return dict(sorted(latest.items()))

    def _select_rows_for_report(self, rows: list[ResultRow]) -> list[ResultRow]:
        comparison_rows = self._select_latest_comparison_block(rows)

        if comparison_rows:
            selected_rows = comparison_rows
        else:
            latest_sample_count = rows[-1].sample_count
            selected_rows = [row for row in rows if row.sample_count == latest_sample_count]

        latest_by_key: dict[tuple[str, int], ResultRow] = {}

        for row in selected_rows:
            latest_by_key[(row.mode, row.threads)] = row

        return sorted(
            latest_by_key.values(),
            key=lambda row: (row.mode != "Serial", row.threads, row.index),
        )

    def _select_latest_comparison_block(self, rows: list[ResultRow]) -> list[ResultRow]:
        for start_index in range(len(rows) - 1, -1, -1):
            first_row = rows[start_index]

            if not self._is_comparison_row(first_row) or first_row.mode != "Serial":
                continue

            block = [first_row]

            for next_row in rows[start_index + 1 :]:
                if not self._is_comparison_row(next_row):
                    break

                if next_row.mode == "Serial":
                    break

                if next_row.sample_count != first_row.sample_count:
                    break

                block.append(next_row)

            return block

        return []

    @staticmethod
    def _is_comparison_row(row: ResultRow) -> bool:
        return row.speedup is not None and row.efficiency is not None

    @staticmethod
    def _parse_optional_float(value: str) -> float | None:
        text = value.strip()
        if not text:
            return None
        return float(text)

    @staticmethod
    def _parse_optional_int(value: str) -> int | None:
        text = value.strip()
        if not text:
            return None
        return int(text)

    @staticmethod
    def _parse_sample_times(value: str) -> list[float]:
        text = value.strip()
        if not text:
            return []
        return [float(part) for part in text.split(";") if part]
