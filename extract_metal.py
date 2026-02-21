import Quartz
from Foundation import NSURL

pdf_url = NSURL.fileURLWithPath_('/Users/okji/dev/games/GeneralsGameCode/Platform/MacOS/docs/reference/dx8_metal_specs/Metal-Shading-Language-Specification.pdf')
pdf_doc = Quartz.PDFDocument.alloc().initWithURL_(pdf_url)

if pdf_doc:
    pages = pdf_doc.pageCount()
    with open('/Users/okji/dev/games/GeneralsGameCode/Platform/MacOS/docs/reference/dx8_metal_specs/metal_spec_extracted.txt', 'w', encoding='utf-8') as f:
        for i in range(pages):
            page = pdf_doc.pageAtIndex_(i)
            text = page.string()
            
            if i > 0:
                f.write("\n")
            f.write("=" * 80 + "\n")
            f.write(f"=== PAGE {i + 1} ===\n")
            f.write("=" * 80 + "\n")
            
            if text:
                f.write(text + "\n")

    print(f"Extracted {pages} pages.")
else:
    print("Failed to load PDF.")
