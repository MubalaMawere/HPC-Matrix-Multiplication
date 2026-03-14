from __future__ import annotations

import csv
from pathlib import Path

from charts_and_graphs.models import ChartDataFiles, ResultRow


class ChartDataWriter:
    def __init__(self, output_dir: Path) -> None:
        self.output_dir = output_dir
        self.output_dir.mkdir(parents=True, exist_ok=True)

    def write_chart_data(self, grouped_rows: dict[int, list[ResultRow]]) -> ChartDataFiles:
        data_files = ChartDataFiles(
            summary_csv=self.output_dir / "summary_chart_data.csv",
            sample_csv=self.output_dir / "sample_chart_points.csv",
        )

        self._write_summary_csv(data_files.summary_csv, grouped_rows)
        self._write_sample_csv(data_files.sample_csv, grouped_rows)
        return data_files

    def _write_summary_csv(self, csv_path: Path, grouped_rows: dict[int, list[ResultRow]]) -> None:
        with csv_path.open("w", newline="", encoding="utf-8") as csv_file:
            writer = csv.writer(csv_file)
            writer.writerow(
                [
                    "matrix_size",
                    "mode",
                    "threads",
                    "sample_count",
                    "average_time_seconds",
                    "speedup",
                    "efficiency",
                    "verified",
                ]
            )

            for matrix_size, rows in grouped_rows.items():
                for row in rows:
                    writer.writerow(
                        [
                            matrix_size,
                            row.mode,
                            row.threads,
                            row.sample_count if row.sample_count is not None else "",
                            f"{row.execution_seconds:.6f}",
                            self._format_optional_float(row.speedup),
                            self._format_optional_float(row.efficiency),
                            row.verified or "",
                        ]
                    )

    def _write_sample_csv(self, csv_path: Path, grouped_rows: dict[int, list[ResultRow]]) -> None:
        with csv_path.open("w", newline="", encoding="utf-8") as csv_file:
            writer = csv.writer(csv_file)
            writer.writerow(
                [
                    "matrix_size",
                    "mode",
                    "threads",
                    "sample_count",
                    "sample_number",
                    "sample_time_seconds",
                ]
            )

            for matrix_size, rows in grouped_rows.items():
                for row in rows:
                    for sample_number, sample_time in enumerate(row.sample_times, start=1):
                        writer.writerow(
                            [
                                matrix_size,
                                row.mode,
                                row.threads,
                                row.sample_count if row.sample_count is not None else "",
                                sample_number,
                                f"{sample_time:.6f}",
                            ]
                        )

    @staticmethod
    def _format_optional_float(value: float | None) -> str:
        if value is None:
            return ""
        return f"{value:.6f}"
