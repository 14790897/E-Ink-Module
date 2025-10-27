// ============================================================================
// API 基础路径
// ============================================================================
const API_BASE = '';

// ============================================================================
// 页面加载时初始化
// ============================================================================
document.addEventListener('DOMContentLoaded', function() {
    loadStatus();
    loadBookList();

    // 文件选择事件
    document.getElementById('fileInput').addEventListener('change', function(e) {
        const fileName = e.target.files[0] ? e.target.files[0].name : '未选择文件';
        document.getElementById('fileName').textContent = fileName;
    });

    // 定期刷新状态（每3秒）
    setInterval(loadStatus, 3000);
});

// ============================================================================
// 加载设备状态
// ============================================================================
async function loadStatus() {
    try {
        const response = await fetch(API_BASE + '/api/status');
        const data = await response.json();

        document.getElementById('currentPage').textContent = data.currentPage || 0;
        document.getElementById('totalPages').textContent = data.totalPages || 0;
        document.getElementById('currentBook').textContent = data.currentBook || '无';
        document.getElementById('storage').textContent = data.storage || '未知';
    } catch (error) {
        console.error('加载状态失败:', error);
    }
}

// ============================================================================
// 加载书籍列表
// ============================================================================
async function loadBookList() {
    try {
        const response = await fetch(API_BASE + '/api/books');
        const books = await response.json();

        const bookList = document.getElementById('bookList');

        if (!books || books.length === 0) {
            bookList.innerHTML = '<div style="text-align: center; color: #999; padding: 20px;">暂无书籍，请上传</div>';
            return;
        }

        bookList.innerHTML = books.map(book => `
            <div class="book-item">
                <div class="book-info">
                    <div class="book-title">${book.name}</div>
                    <div class="book-meta">${book.size} | ${book.pages || 0} 页</div>
                </div>
                <div class="book-actions">
                    <button class="btn btn-primary" onclick="readBook('${book.name}')">阅读</button>
                    <button class="btn btn-danger" onclick="deleteBook('${book.name}')">删除</button>
                </div>
            </div>
        `).join('');

    } catch (error) {
        console.error('加载书籍列表失败:', error);
        document.getElementById('bookList').innerHTML = '<div style="text-align: center; color: #f00; padding: 20px;">加载失败</div>';
    }
}

// ============================================================================
// 上传书籍
// ============================================================================
async function uploadBook() {
    const fileInput = document.getElementById('fileInput');
    const file = fileInput.files[0];

    if (!file) {
        alert('请先选择文件');
        return;
    }

    // 检查文件类型
    if (!file.name.endsWith('.txt')) {
        alert('只支持TXT格式文件');
        return;
    }

    // 检查文件大小（限制为2MB）
    if (file.size > 2 * 1024 * 1024) {
        alert('文件大小不能超过2MB');
        return;
    }

    showLoading(true);

    try {
        const formData = new FormData();
        formData.append('file', file);

        const response = await fetch(API_BASE + '/api/upload', {
            method: 'POST',
            body: formData
        });

        const result = await response.json();

        if (result.success) {
            alert('上传成功！');
            fileInput.value = '';
            document.getElementById('fileName').textContent = '未选择文件';
            loadBookList();
            loadStatus();
        } else {
            alert('上传失败：' + (result.message || '未知错误'));
        }
    } catch (error) {
        console.error('上传失败:', error);
        alert('上传失败：' + error.message);
    } finally {
        showLoading(false);
    }
}

// ============================================================================
// 阅读书籍
// ============================================================================
async function readBook(bookName) {
    showLoading(true);

    try {
        const response = await fetch(API_BASE + '/api/read', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ book: bookName, page: 0 })
        });

        const result = await response.json();

        if (result.success) {
            alert(`开始阅读《${bookName}》`);
            loadStatus();
        } else {
            alert('打开失败：' + (result.message || '未知错误'));
        }
    } catch (error) {
        console.error('打开书籍失败:', error);
        alert('打开失败：' + error.message);
    } finally {
        showLoading(false);
    }
}

// ============================================================================
// 删除书籍
// ============================================================================
async function deleteBook(bookName) {
    if (!confirm(`确定要删除《${bookName}》吗？`)) {
        return;
    }

    showLoading(true);

    try {
        const response = await fetch(API_BASE + '/api/delete', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ book: bookName })
        });

        const result = await response.json();

        if (result.success) {
            alert('删除成功！');
            loadBookList();
            loadStatus();
        } else {
            alert('删除失败：' + (result.message || '未知错误'));
        }
    } catch (error) {
        console.error('删除书籍失败:', error);
        alert('删除失败：' + error.message);
    } finally {
        showLoading(false);
    }
}

// ============================================================================
// 翻页控制
// ============================================================================
async function nextPage() {
    showLoading(true);

    try {
        const response = await fetch(API_BASE + '/api/next', {
            method: 'POST'
        });

        const result = await response.json();

        if (result.success) {
            loadStatus();
        } else {
            alert('翻页失败：' + (result.message || '未知错误'));
        }
    } catch (error) {
        console.error('翻页失败:', error);
        alert('翻页失败：' + error.message);
    } finally {
        showLoading(false);
    }
}

async function prevPage() {
    showLoading(true);

    try {
        const response = await fetch(API_BASE + '/api/prev', {
            method: 'POST'
        });

        const result = await response.json();

        if (result.success) {
            loadStatus();
        } else {
            alert('翻页失败：' + (result.message || '未知错误'));
        }
    } catch (error) {
        console.error('翻页失败:', error);
        alert('翻页失败：' + error.message);
    } finally {
        showLoading(false);
    }
}

// ============================================================================
// 显示/隐藏加载动画
// ============================================================================
function showLoading(show) {
    const loading = document.getElementById('loading');
    if (show) {
        loading.classList.add('show');
    } else {
        loading.classList.remove('show');
    }
}

// ============================================================================
// 工具函数：格式化文件大小
// ============================================================================
function formatFileSize(bytes) {
    if (bytes < 1024) return bytes + ' B';
    if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(2) + ' KB';
    return (bytes / (1024 * 1024)).toFixed(2) + ' MB';
}
