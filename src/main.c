/* ============================================================
   SISTEMA DE RECOMENDACAO EM GRAFOS — EDA2
   Implementacao em C (C99) com pipeline algoritmico completo
   + Fase 0 de PLN: TF-IDF, projecao texto-texto por cosseno
   ------------------------------------------------------------
   Compilar:  make
   Executar:  ./recomendador_app
   ============================================================ */

#include "digrafo.h"
#include "vocab.h"
#include "algoritmos.h"
#include "recomendador.h"
#include "relatorio.h"

int main(void)
{
   Digrafo g;
   inicializar_digrafo(&g);

   /* ── Vertices ── */

   /* ── Usuarios ── */
   int u1 = adicionar_vertice(&g, "u1", USUARIO); /* Joao */
   int u2 = adicionar_vertice(&g, "u2", USUARIO); /* Maria */
   int u3 = adicionar_vertice(&g, "u3", USUARIO); /* Pedro */

   /* ── Livros ── */
   int t1 = adicionar_vertice(&g, "t1", TEXTO); /* Dom Casmurro */
   int t2 = adicionar_vertice(&g, "t2", TEXTO); /* Memorias Postumas de Bras Cubas */
   int t3 = adicionar_vertice(&g, "t3", TEXTO); /* Romeu e Julieta */
   int t4 = adicionar_vertice(&g, "t4", TEXTO); /* A Ilha do Tesouro */
   int t5 = adicionar_vertice(&g, "t5", TEXTO); /* O Conde de Monte Cristo */

   /* ── Generos ── */
   int fi = adicionar_vertice(&g, "ficcao", GENERO);
   int av = adicionar_vertice(&g, "aventura", GENERO);
   int ro = adicionar_vertice(&g, "romance", GENERO);
   int th = adicionar_vertice(&g, "thriller", GENERO);

   /* ── Conteudo textual (DADOS SINTETICOS) ──
      Resenhas ficticias de classicos da literatura, geradas por LLM,
      em portugues sem acentos (corpus ASCII para tokenizacao limpa).
      Plataforma: leitura digital com resenhas em texto livre.
    */

   /* Dom Casmurro — amor/tragedia */
   definir_conteudo(&g, t1,
                    "amor e paixao definem esta historia de ciume e traicao. o ciume "
                    "corroi o coracao e a traicao leva o amor a tragedia. o destino do "
                    "casal termina em morte e sofrimento. uma paixao marcada pelo ciume "
                    "pela duvida e pela dor do amor traido.");

   /* Memorias Postumas de Bras Cubas — amor/tragedia */
   definir_conteudo(&g, t2,
                    "um amor proibido e uma paixao adultera conduzem a traicao. o ciume e "
                    "a culpa cercam o amor enquanto a morte se aproxima. a tragedia revela "
                    "o destino e o sofrimento de um coracao dividido entre o amor e a "
                    "perda. paixao traicao e morte.");

   /* Romeu e Julieta — amor/tragedia */
   definir_conteudo(&g, t3,
                    "o amor proibido e a paixao ardente terminam em tragedia. a traicao do "
                    "destino separa o casal e o ciume alimenta o odio. o amor verdadeiro "
                    "encontra a morte e o coracao mergulha no sofrimento. uma paixao eterna "
                    "desfeita pela tragedia e pela morte.");

   /* A Ilha do Tesouro — aventura pura */
   definir_conteudo(&g, t4,
                    "uma aventura pelo mar em busca de um tesouro perdido. a viagem "
                    "enfrenta o perigo a batalha e a coragem. o heroi vence o inimigo "
                    "supera o perigo e conquista o tesouro. aventura coragem e perigo no "
                    "mar em uma viagem de batalha.");

   /* O Conde de Monte Cristo — vinganca + amor (ponte) */
   definir_conteudo(&g, t5,
                    "uma aventura de vinganca nascida da traicao e do amor perdido. a "
                    "paixao e o ciume movem um coracao ferido enquanto a busca por um "
                    "tesouro leva o heroi ao mar. vinganca traicao e amor caminham juntos "
                    "nesta aventura de coragem e destino.");

   /* ── Perfis tematicos dos GENEROS (palavras-chave) ──
      Pequenos documentos que posicionam cada genero no mesmo espaco
      vetorial dos livros. As arestas Texto->Genero serao descobertas
      automaticamente por similaridade cosseno, nao mais hardcoded.
    */

   definir_conteudo(&g, fi, "amor tragedia destino ciume coracao");
   definir_conteudo(&g, av, "aventura mar tesouro viagem perigo coragem heroi batalha");
   definir_conteudo(&g, ro, "amor paixao coracao sofrimento casal");
   definir_conteudo(&g, th, "traicao vinganca morte perigo inimigo destino");

   /* ── Leituras: Usuario -> Texto (peso = nota) ── */
   adicionar_arco(&g, u1, t1, 5.0);
   adicionar_arco(&g, u1, t2, 4.0);
   adicionar_arco(&g, u1, t3, 2.0);
   adicionar_arco(&g, u2, t1, 3.0);
   adicionar_arco(&g, u2, t4, 5.0);
   adicionar_arco(&g, u3, t3, 4.0);
   adicionar_arco(&g, u3, t5, 5.0);

   /* ── Categorizacao Texto -> Genero: agora DINAMICA (ver Fase 0).
      Os antigos vinculos manuais foram removidos e sao substituidos
      por construir_ligacoes_genero(), baseada em TF-IDF + cosseno.  */

   /* ── FASE 0: PLN (TF-IDF) e projecao texto-texto ── */
   Vocabulario vocab;
   inicializar_vocab(&vocab);

   static int tf_cont[MAX_VERTICES][MAX_VOCAB];
   static double tfidf[MAX_VERTICES][MAX_VOCAB];
   static double cos_mat[MAX_VERTICES][MAX_VERTICES];
   int total_tokens[MAX_VERTICES];

   int num_docs = construir_vocab_e_contagens(&g, &vocab, tf_cont, total_tokens);
   calcular_tfidf(&g, &vocab, num_docs, tf_cont, total_tokens, tfidf);
   int arestas_genero = construir_ligacoes_genero(&g, &vocab, tfidf);
   construir_projecao_texto(&g, &vocab, tfidf, cos_mat);

   /* ── Relatorios e Recomendacoes ── */
   imprimir_relatorio_fase0(&g, &vocab, num_docs, arestas_genero, tfidf, cos_mat);

   /* ── Recomendacoes para u1 ── */
   Recomendacao resultado[MAX_VERTICES];
   int usou_fallback = 0;
   int n = recomendar(&g, u1, resultado, 5, cos_mat, &usou_fallback);

   imprimir_recomendacoes(&g, resultado, n, usou_fallback);

   /* Suprimir warnings de variaveis nao utilizadas nos dados sinteticos */
   (void)u2;
   (void)u3;
   (void)t2;
   (void)t3;
   (void)t4;
   (void)t5;
   (void)fi;
   (void)av;
   (void)ro;
   (void)th;

   liberar_digrafo(&g);
   return 0;
}
