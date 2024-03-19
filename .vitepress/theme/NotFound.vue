<script setup>
import { useRouter } from 'vitepress';
import { repo_url } from './params';

const router = useRouter();
if (router.route.data.isNotFound)
    window.location.replace(repo_url + 'blob/main' + router.route.path);
</script>