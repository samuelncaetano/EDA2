# Sistema de Recomendação em Grafos — EDA2

Sistema de recomendação de textos (livros) baseado em **grafos dirigidos** (digrafos), combinando algoritmos clássicos de grafos com **Processamento de Linguagem Natural (PLN)** via TF-IDF. Desenvolvido em **C (C99)** como trabalho da disciplina de Estruturas de Dados e Algoritmos 2.

---

## 1. Sobre o Projeto

### 1.1. O Problema

Dado um conjunto de **Usuários**, **Textos** (livros) e **Gêneros literários**, o sistema deve recomendar textos ainda não lidos por um usuário-alvo. A recomendação precisa considerar múltiplos sinais — distância no grafo, similaridade semântica, sobreposição de perfil com outros leitores e afinidade por gênero — e **explicar** ao usuário o porquê de cada recomendação.

### 1.2. Modelagem como Grafo

O domínio é modelado como um **digrafo ponderado** com três tipos de vértice e dois tipos de aresta:

```
Usuário ──(nota)──▶ Texto ──(cosseno)──▶ Gênero
                      │
               (projeção T↔T)
                      │
                    Texto
```

| Vértice   | Representa                           |
| --------- | ------------------------------------ |
| `USUARIO` | Leitor que avaliou textos            |
| `TEXTO`   | Livro com conteúdo textual (resenha) |
| `GENERO`  | Categoria literária                  |

| Aresta          | Tipo     | Peso                        |
| --------------- | -------- | --------------------------- |
| Usuário → Texto | Normal   | Nota dada pelo leitor (1–5) |
| Texto → Gênero  | Normal   | Similaridade cosseno TF-IDF |
| Texto ↔ Texto   | Projeção | Similaridade cosseno TF-IDF |

### 1.3. Pipeline de Solução

O sistema executa um **pipeline de 5 fases**, cada uma baseada em um algoritmo

ou técnica distinta:

| Fase | Nome                        | Algoritmo / Técnica           | Saída                           |
| ---- | --------------------------- | ----------------------------- | ------------------------------- |
| 0    | PLN e Projeção Semântica    | TF-IDF + Similaridade Cosseno | Arestas T→G e T↔T automáticas   |
| 1    | Descoberta de Candidatos    | BFS com lookup inverso        | Lista de textos candidatos      |
| 2    | Distância no Grafo          | Dijkstra (pesos invertidos)   | Score `s_dist` por candidato    |
| 3    | Afinidade por Gênero        | Kahn (ordenação topológica)   | Score `s_topo` por candidato    |
| 4    | Similaridade entre Usuários | Índice de Jaccard             | Score `s_jaccard` por candidato |

O **score final** de cada candidato é a soma de 5 componentes:

```
score_total = s_dist + s_caminhos + s_jaccard + s_topo + s_tfidf
```

#### 1.3.1. Detalhamento das Fases

**Fase 0 — PLN (TF-IDF + Cosseno)**
- Tokeniza o conteúdo textual de cada livro e gênero, removendo **stopwords**.
- Constrói um vocabulário global com **hashing (djb2)** e sondagem linear.
- Calcula vetores **TF-IDF normalizados** (norma L2) para cada documento.
- Gera arestas **Texto → Gênero** (limiar `> 0.15`) e **Texto ↔ Texto**
  (limiar `> 0.10`) por similaridade cosseno, eliminando a necessidade de
  categorização manual.

**Fase 1 — BFS com Lookup Inverso**
- A partir do usuário-alvo, percorre o grafo via **BFS** seguindo arestas normais.
- Ao alcançar um vértice de Gênero, faz um **lookup inverso** para encontrar
  textos daquele gênero que o usuário ainda não leu.
- Se nenhum candidato for encontrado, ativa o **fallback semântico**: percorre
  arestas de projeção T↔T a partir dos livros já lidos.

**Fase 2 — Dijkstra (Pesos Invertidos)**
- Executa **Dijkstra** a partir do usuário-alvo, usando `1/peso` como custo
  (notas altas → distância curta).
- Gera `s_dist = 1/distância` — textos mais “próximos” no grafo recebem
  score maior.

**Fase 3 — Kahn (Ordenação Topológica)**
- Calcula a **afinidade** do usuário por cada gênero: `Σ(nota × peso_aresta_T→G)`.
- Usa o **algoritmo de Kahn** para ordenar os gêneros por afinidade.
- `s_topo = 1/(posição + 1)` — textos do gênero favorito recebem score maior.

**Fase 4 — Índice de Jaccard**
- Para cada par (usuário-alvo, outro usuário), calcula o **índice de Jaccard**
  sobre os gêneros compartilhados.
- `s_jaccard` acumula a similaridade Jaccard de todos os usuários que também
  leram o texto candidato.

---

## 2. Como Compilar e Executar

### 2.1. Pré-requisitos

| Ferramenta | Versão Mínima |
| ---------- | ------------- |
| GCC        | 4.9+ (C99)    |
| CMake      | 3.10+         |
| Make       | qualquer      |

### 2.2. Compilação

```bash
# 1. Clone o repositório (se necessário)
git clone https://github.com/samuelncaetano/EDA2.git
cd EDA2

# 2. Crie o diretório de build e compile
mkdir -p build
cd build
cmake ..
make
```

> [!TIP]
> Para recompilar após alterações no código, basta executar `make` dentro do diretório `build/` — o CMake detecta mudanças automaticamente.

### 2.3. Execução

```bash
# Dentro do diretório build/
./recomendador
```

O programa não recebe argumentos nem entrada interativa. Todo o grafo (usuários, textos, gêneros, notas e resenhas) está definido como **dados sintéticos** diretamente no `main.c`. A saída completa é impressa no terminal.

### 2.4. Estrutura do Projeto

```
EDA2/
├── CMakeLists.txt          # Configuração de build (CMake)
├── include/                # Headers (.h)
│   ├── algoritmos.h        # BFS, Dijkstra, Kahn, Jaccard
│   ├── digrafo.h           # Estrutura do digrafo
│   ├── recomendador.h      # Motor de recomendação e scores
│   ├── relatorio.h         # Impressão dos relatórios
│   └── vocab.h             # Vocabulário, TF-IDF, cosseno
├── src/                    # Implementações (.c)
│   ├── main.c              # Ponto de entrada e dados sintéticos
│   ├── digrafo.c           # Operações sobre o digrafo
│   ├── vocab.c             # Tokenização, TF-IDF, projeções
│   ├── algoritmos.c        # Algoritmos de grafos
│   ├── recomendador.c      # Lógica de scoring e recomendação
│   └── relatorio.c         # Formatação da saída explicável
└── build/                  # Diretório de build (gerado)
```

---

## 3. Interpretação da Saída

O programa imprime duas seções principais no terminal. Abaixo, explicamos **cada dado** e como interpretá-lo.

---

### 3.1. Seção 1: FASE 0 — PLN: TF-IDF e Projeção Texto-Texto

```
==================================================
 FASE 0 — PLN: TF-IDF e PROJECAO TEXTO-TEXTO
==================================================
Documentos processados : 9
Termos no vocabulario  : 64
Limiar theta (T-T)     : 0.10
Limiar genero (T-G)    : 0.15
```

| Campo                  | Significado                                                            |
| ---------------------- | ---------------------------------------------------------------------- |
| Documentos processados | Quantidade de documentos analisados (5 textos + 4 gêneros = 9)         |
| Termos no vocabulário  | Palavras únicas restantes após remoção de stopwords                    |
| Limiar theta (T-T)     | Similaridade mínima para criar aresta de projeção entre dois textos    |
| Limiar gênero (T-G)    | Similaridade mínima para associar um texto a um gênero automaticamente |

#### 3.1.1. Matriz Texto × Gênero

```
Categorizacao DINAMICA Texto -> Genero:
Matriz de similaridade cosseno (texto x genero):
        ficcao    aventura  romance   thriller
  Dom C 0.250     0.000     0.245     0.070
  ...
```

**Como ler:** cada célula mostra a **similaridade cosseno** (0 a 1) entre o vetor TF-IDF do texto (linha) e o vetor TF-IDF do gênero (coluna).

- **Valor alto (ex: 0.778)** → o texto tem forte semelhança vocabular com o
  gênero. Uma aresta Texto → Gênero será criada.
- **Valor zero (0.000)** → nenhuma sobreposição de vocabulário relevante.
- **Valores acima do limiar (0.15)** geram arestas no grafo.

> [!NOTE]
> Essa categorização é **dinâmica**: os gêneros de cada livro são descobertos automaticamente pelo TF-IDF, e não atribuídos manualmente.

#### 3.1.2. Arestas Texto → Gênero Criadas

```
Arestas T->G criadas (cosseno > 0.15): 10
  Dom Casmurro -> romance   : 0.245
  Dom Casmurro -> ficcao    : 0.250
  ...
```

Lista todas as arestas que o sistema criou após aplicar o limiar. O número após `:` é o peso da aresta (= similaridade cosseno). **Um texto pode pertencer a múltiplos gêneros** — ex: “Dom Casmurro” pertence a ficção *e* romance.

#### 3.1.3. Matriz Texto × Texto

```
Matriz de similaridade cosseno (texto x texto):
        Dom C   Memor   Romeu   Ilha    Conde
  Dom C 0.000   0.139   0.261   0.031   0.105
  ...
```

**Como ler:** cada célula mostra a **similaridade semântica** entre dois textos, calculada pelo cosseno de seus vetores TF-IDF.

- A diagonal é sempre **0.000** (um texto comparado consigo mesmo não é
  relevante para recomendação).
- **Valores altos** indicam textos com vocabulário/temática semelhante.
  Ex: Dom Casmurro ↔ Romeu e Julieta = 0.261 (ambos tratam de amor e
  tragédia).
- **Valores baixos ou zero** indicam temas distintos. Ex: Memórias Póstumas
  ↔ Ilha do Tesouro = 0.000.

#### 3.1.4. Arestas de Projeção Texto ↔ Texto

```
Arestas de projecao T->T criadas (cosseno > theta):
  Dom Casmurro <-> Memorias Postumas de Bras Cubas : 0.139
  Dom Casmurro <-> Romeu e Julieta : 0.261
  ...
```

Arestas **bidirecionais** criadas entre textos cuja similaridade cosseno supera o limiar `theta = 0.10`. Essas arestas alimentam o **fallback semântico** (Fase 1) e o score `s_tfidf`.

---

### 3.2. Seção 2: RECOMENDAÇÕES

```
==================================================
 RECOMENDACOES PARA: u1
==================================================
[BFS por genero nao achou candidatos — usando FALLBACK
 SEMANTICO via projecao T->T sobre os textos lidos]
```

Se esta mensagem aparece, significa que a **BFS por gênero** não encontrou textos candidatos (todos os textos dos gêneros alcançados já foram lidos pelo usuário), então o sistema recorre ao **fallback semântico**: busca textos vizinhos pelas arestas de projeção T↔T.

#### 3.2.1. Score de cada Recomendação

```
[1] Texto : O Conde de Monte Cristo | Score Total: 1.1182
    Dijkstra  (s_dist)    : 0.0000
    BFS       (s_caminhos): 0.2000
    Jaccard   (s_jaccard) : 0.5000
    Kahn      (s_topo)    : 0.3333
    TF-IDF    (s_tfidf)   : 0.0848
```

O score total é a **soma dos 5 componentes**. Cada um representa uma “razão”

diferente para recomendar o texto:

| Componente            | Fórmula                                       | Interpretação                                                                                        |
| --------------------- | --------------------------------------------- | ---------------------------------------------------------------------------------------------------- |
| `s_dist` (Dijkstra)   | `1 / distância_mínima`                        | Quão **próximo** o texto está do usuário no grafo. Maior = mais próximo.                             |
| `s_caminhos` (BFS)    | `num_caminhos × 0.1`                          | Por quantos **caminhos distintos** o usuário chega ao texto. Maior = mais acessível.                 |
| `s_jaccard` (Jaccard) | `Σ Jaccard(u_alvo, u_i)` se `u_i` leu o texto | Outros **leitores similares** também leram este texto. Maior = recomendação colaborativa mais forte. |
| `s_topo` (Kahn)       | `1 / (posição_gênero + 1)`                    | O texto pertence a um **gênero favorito** do usuário. Maior = gênero preferido.                      |
| `s_tfidf` (TF-IDF)    | `média(cosseno(candidato, lido_i))`           | O texto é **semanticamente similar** aos textos já lidos. Maior = tema mais próximo.                 |

#### 3.2.2. Exemplo Prático de Leitura

Para o resultado mostrado acima:

- **`s_dist = 0.0000`** → O texto não é alcançável por arestas normais a
  partir do usuário (distância infinita). Dijkstra não contribui.
- **`s_caminhos = 0.2000`** → O texto foi encontrado por **2 caminhos**
  (via projeção semântica, no fallback). Valor = 2 × 0.1.
- **`s_jaccard = 0.5000`** → Outro(s) usuário(s) que leram “O Conde de Monte
  Cristo” têm **50% de sobreposição de gêneros** com o Usuário 1.
- **`s_topo = 0.3333`** → O gênero do texto está na **3ª posição** do ranking
  de afinidade. Valor = 1/(2+1) = 0.333.
- **`s_tfidf = 0.0848`** → Similaridade semântica **média** entre “O Conde de
  Monte Cristo” e os livros já lidos pelo Usuário 1.

---

## 4. Algoritmos e Complexidades

| Algoritmo              | Complexidade    | Uso no projeto                              |
| ---------------------- | --------------- | ------------------------------------------- |
| Hash (djb2) + sondagem | O(1) amortizado | Indexação do vocabulário                    |
| TF-IDF + Cosseno       | O(D × V)        | Vetorização e similaridade entre documentos |
| BFS                    | O(V + E)        | Descoberta de candidatos por gênero         |
| Dijkstra               | O(V²)           | Distância mínima no grafo ponderado         |
| Kahn (Topológico)      | O(G²)           | Ranking de gêneros por afinidade            |
| Jaccard                | O(U × V)        | Similaridade entre perfis de usuários       |

Onde `V` = vértices, `E` = arestas, `D` = documentos, `G` = gêneros, `U` = usuários.
