// ============================================================================
// API 基础路径
// ============================================================================
const API_BASE = '';

// ============================================================================
// 提示框函数
// ============================================================================
function showToast(message, type = 'info', duration = 2000) {
    const container = document.getElementById('toastContainer');

    // 创建遮罩层
    const overlay = document.createElement('div');
    overlay.className = 'toast-overlay';

    // 创建提示框
    const toast = document.createElement('div');
    toast.className = `toast ${type}`;

    // 图标映射
    const icons = {
        success: '✓',
        error: '✕',
        info: 'ℹ',
        warning: '⚠'
    };

    toast.innerHTML = `
        <div class="toast-icon">${icons[type] || icons.info}</div>
        <div class="toast-message">${message}</div>
        <div class="toast-buttons">
            <button class="btn btn-primary" onclick="closeToast(this)">确定</button>
        </div>
    `;

    container.appendChild(overlay);
    container.appendChild(toast);

    // 点击遮罩层关闭
    overlay.onclick = () => closeToast(toast);

    // 自动关闭（如果设置了持续时间）
    if (duration > 0 && type !== 'error') {
        setTimeout(() => closeToast(toast), duration);
    }
}

function showConfirm(message, onConfirm, onCancel) {
    const container = document.getElementById('toastContainer');

    // 创建遮罩层
    const overlay = document.createElement('div');
    overlay.className = 'toast-overlay';

    // 创建提示框
    const toast = document.createElement('div');
    toast.className = 'toast warning';

    toast.innerHTML = `
        <div class="toast-icon">⚠</div>
        <div class="toast-message">${message}</div>
        <div class="toast-buttons">
            <button class="btn btn-secondary" id="cancelBtn">取消</button>
            <button class="btn btn-danger" id="confirmBtn">确定</button>
        </div>
    `;

    container.appendChild(overlay);
    container.appendChild(toast);

    // 绑定按钮事件
    const confirmBtn = toast.querySelector('#confirmBtn');
    const cancelBtn = toast.querySelector('#cancelBtn');

    confirmBtn.onclick = () => {
        closeToast(toast);
        if (onConfirm) onConfirm();
    };

    cancelBtn.onclick = () => {
        closeToast(toast);
        if (onCancel) onCancel();
    };

    // 点击遮罩层取消
    overlay.onclick = () => {
        closeToast(toast);
        if (onCancel) onCancel();
    };
}

function closeToast(element) {
    const toast = element.classList?.contains('toast') ? element : element.closest('.toast');
    if (!toast) return;

    const overlay = toast.previousElementSibling;

    // 添加淡出动画
    toast.classList.add('fade-out');
    if (overlay) overlay.classList.add('fade-out');

    // 动画结束后移除元素
    setTimeout(() => {
        if (toast.parentNode) toast.parentNode.removeChild(toast);
        if (overlay && overlay.parentNode) overlay.parentNode.removeChild(overlay);
    }, 300);
}


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
        showToast('请先选择文件', 'warning', 3000);
        return;
    }

    // 检查文件类型
    if (!file.name.endsWith('.txt')) {
        showToast('只支持TXT格式文件', 'error', 3000);
        return;
    }

    // 检查文件大小（限制为2MB）
    if (file.size > 2 * 1024 * 1024) {
        showToast('文件大小不能超过2MB', 'error', 3000);
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
            showToast('上传成功！', 'success', 2000);
            fileInput.value = '';
            document.getElementById('fileName').textContent = '未选择文件';
            loadBookList();
            loadStatus();
        } else {
            showToast('上传失败：' + (result.message || '未知错误'), 'error', 3000);
        }
    } catch (error) {
        console.error('上传失败:', error);
        showToast('上传失败：' + error.message, 'error', 3000);
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
            showToast(`开始阅读《${bookName}》`, 'success', 2000);
            loadStatus();
        } else {
            showToast('打开失败：' + (result.message || '未知错误'), 'error', 3000);
        }
    } catch (error) {
        console.error('打开书籍失败:', error);
        showToast('打开失败：' + error.message, 'error', 3000);
    } finally {
        showLoading(false);
    }
}

// ============================================================================
// 删除书籍
// ============================================================================
async function deleteBook(bookName) {
    showConfirm(`确定要删除《${bookName}》吗？`, async () => {
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
                showToast('删除成功！', 'success', 2000);
                loadBookList();
                loadStatus();
            } else {
                showToast('删除失败：' + (result.message || '未知错误'), 'error', 3000);
            }
        } catch (error) {
            console.error('删除书籍失败:', error);
            showToast('删除失败：' + error.message, 'error', 3000);
        } finally {
            showLoading(false);
        }
    });
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
            showToast('翻页失败：' + (result.message || '未知错误'), 'error', 3000);
        }
    } catch (error) {
        console.error('翻页失败:', error);
        showToast('翻页失败：' + error.message, 'error', 3000);
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
            showToast('翻页失败：' + (result.message || '未知错误'), 'error', 3000);
        }
    } catch (error) {
        console.error('翻页失败:', error);
        showToast('翻页失败：' + error.message, 'error', 3000);
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
