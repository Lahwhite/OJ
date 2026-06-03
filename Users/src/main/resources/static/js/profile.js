document.addEventListener('DOMContentLoaded', function() {
    const avatarPreview = document.getElementById('avatarPreview');
    const avatarInput = document.getElementById('avatarInput');

    avatarPreview.addEventListener('error', function () {
        this.onerror = null; // 防止默认头像也加载失败时死循环
        this.src = '/images/default-avatar.png';
    });

    function previewImage(event) {
        const file = event.target.files[0];
        if (!file)
            return;

        const reader = new FileReader();
        reader.onload = function(e) {
            avatarPreview.src = e.target.result;
        };
        reader.readAsDataURL(file);
    }

    avatarInput.addEventListener('change', previewImage);
});