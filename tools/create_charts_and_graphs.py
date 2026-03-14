#!/usr/bin/env python3

from __future__ import annotations

from pathlib import Path

from charts_and_graphs.chart_builder import ChartBuilder
from charts_and_graphs.chart_data_writer import ChartDataWriter
from charts_and_graphs.chart_image_writer import ChartImageWriter
from charts_and_graphs.csv_loader import ResultDataLoader
from charts_and_graphs.report_builder import ReportBuilder


CSV_PATH = Path("output/performance_results.csv")
OUTPUT_DIR = Path("output/charts")


def main() -> None:
    loader = ResultDataLoader(CSV_PATH)
    grouped_rows = loader.load_latest_rows_by_size()

    if not grouped_rows:
        raise SystemExit("The CSV file does not contain any rows yet.")

    chart_builder = ChartBuilder()
    chart_data_writer = ChartDataWriter(OUTPUT_DIR)
    chart_image_writer = ChartImageWriter(OUTPUT_DIR)
    report_builder = ReportBuilder(OUTPUT_DIR)

    chart_sections = {}
    chart_figures = {}

    for matrix_size, rows in grouped_rows.items():
        built_charts = chart_builder.create_charts(matrix_size, rows)
        chart_sections[matrix_size] = built_charts.sections
        chart_figures[matrix_size] = built_charts.figures

    chart_data_files = chart_data_writer.write_chart_data(grouped_rows)
    chart_image_folders = chart_image_writer.write_chart_images(chart_figures)
    report_path = report_builder.write_report(CSV_PATH, grouped_rows, chart_sections, chart_data_files)
    print(f"Chart report created: {report_path.resolve()}")
    print(f"PNG charts created: {chart_image_folders.png_dir.resolve()}")
    print(f"SVG charts created: {chart_image_folders.svg_dir.resolve()}")


if __name__ == "__main__":
    main()
