from __future__ import annotations

from pathlib import Path

from charts_and_graphs.models import ChartFigures, ChartImageFolders


class ChartImageWriter:
    def __init__(self, output_dir: Path) -> None:
        self.output_dir = output_dir
        self.png_dir = self.output_dir / "png"
        self.svg_dir = self.output_dir / "svg"
        self.png_dir.mkdir(parents=True, exist_ok=True)
        self.svg_dir.mkdir(parents=True, exist_ok=True)

    def write_chart_images(self, figures_by_size: dict[int, ChartFigures]) -> ChartImageFolders:
        for matrix_size, figures in figures_by_size.items():
            self._write_one_matrix_set(matrix_size, figures)

        return ChartImageFolders(png_dir=self.png_dir, svg_dir=self.svg_dir)

    def _write_one_matrix_set(self, matrix_size: int, figures: ChartFigures) -> None:
        base_name = f"matrix_{matrix_size}x{matrix_size}"

        self._write_image(figures.sample_figure, self.png_dir / f"{base_name}_sample_spread.png", 1400, 720)
        self._write_image(figures.sample_figure, self.svg_dir / f"{base_name}_sample_spread.svg", 1400, 720)

        self._write_image(figures.time_figure, self.png_dir / f"{base_name}_average_time.png", 1200, 700)
        self._write_image(figures.time_figure, self.svg_dir / f"{base_name}_average_time.svg", 1200, 700)

        self._write_image(figures.scaling_figure, self.png_dir / f"{base_name}_speedup_gap.png", 1200, 700)
        self._write_image(figures.scaling_figure, self.svg_dir / f"{base_name}_speedup_gap.svg", 1200, 700)

    @staticmethod
    def _write_image(figure, output_path: Path, width: int, height: int) -> None:
        try:
            figure.write_image(output_path, width=width, height=height, scale=2)
        except Exception as error:
            raise SystemExit(
                "Unable to export chart images. Install the Python dependencies from requirements.txt "
                f"so Plotly can write PNG and SVG files. Original error: {error}"
            ) from error
